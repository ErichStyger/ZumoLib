/**
 * \file
 * \brief Implementation of maze solving
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * This module is used to solve a line maze.
 */

#include "platform.h"
#if PL_CONFIG_MAZE_SOLVING
#include "maze.h"
#include "turn.h"
#include "McuShell.h"
#include "McuRTOS.h"
#if PL_CONFIG_USE_LEDS
  #include "leds.h"
  #include "McuLED.h"
#endif
#include "lineFollow.h"
#include "lineHistory.h"
#include "McuEvents.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "pid.h"
#include "drive.h"
#include "reflectance.h"
#include "shell.h"
#if PL_CONFIG_USE_BUZZER
  #include "buzzer.h"
#endif

#define MAZE_DEBUG_PRINT          (0)   /* careful: this will slow down the PID loop frequency! */
#define MAZE_DO_DEBUG_WAIT        (1)   /* if set to one, it will add some debug wait to stop the engines */
#if MAZE_DO_DEBUG_WAIT
  #define MAZE_DO_DEBUG_WAIT_MS  (500) /* waiting time in milliseconds */
#endif

#if MAZE_DO_DEBUG_WAIT /* debug mode */
static void DebugStopAndWait(void) {
  static volatile bool waitHere = FALSE;

  TURN_Turn(TURN_STOP, NULL);
  vTaskDelay(MAZE_DO_DEBUG_WAIT_MS);
  waitHere = TRUE; /* set a breakpoint here */
}
#endif

#define MAZE_MAX_PATH       64    /* maximum number of turns in path */
static TURN_Kind path[MAZE_MAX_PATH];
static uint8_t pathLength;
static bool isSolved = FALSE;
static bool useLeftHandOnTheWallRule = TRUE;

bool Maze_IsLeftHandRule(void) {
  return useLeftHandOnTheWallRule;
}

uint8_t Maze_SetHandRule(bool isLeft) {
#if 0
  const NVMC_RobotData *ptr;
  NVMC_RobotData data;
  uint8_t res;

  res = ERR_OK;
  useLeftHandOnTheWallRule = isLeft;
  ptr = NVMC_GetRobotData();
  if (ptr!=NULL && ptr->MazeLeftHandOnTheWall!=useLeftHandOnTheWallRule) {
    data = *NVMC_GetRobotData();
    res = NVMC_SaveRobotData(&data);
  }
#else
  useLeftHandOnTheWallRule = isLeft;
#endif
  return ERR_OK;
}

/* used to convert path items while running backwards */
static TURN_Kind MirrorTurn(TURN_Kind turn) {
  switch(turn) {
    case TURN_LEFT90: return TURN_RIGHT90;
    case TURN_RIGHT90: return TURN_LEFT90;
    case TURN_LEFT180: return TURN_STRAIGHT;
    case TURN_RIGHT180: return TURN_STRAIGHT;
    case TURN_STRAIGHT: return TURN_STRAIGHT;
    case TURN_STEP_LINE_FW: return TURN_STEP_LINE_BW;
    case TURN_STEP_LINE_BW: return TURN_STEP_LINE_FW;
    case TURN_STEP_PAST_LINE_FW: return TURN_STEP_PAST_LINE_BW;
    case TURN_STEP_PAST_LINE_BW: return TURN_STEP_PAST_LINE_FW;
    case TURN_STEP_LINE_FW_AND_PAST_LINE: return TURN_STEP_LINE_BW_AND_PAST_LINE;
    case TURN_STEP_LINE_BW_AND_PAST_LINE: return TURN_STEP_LINE_FW_AND_PAST_LINE;
    case TURN_STOP:
    case TURN_FINISH:
    default:
      return TURN_STOP;
  }
}

TURN_Kind Maze_SelectTurnBw(REF_LineKind prev, REF_LineKind curr);
/*!
 * \brief Performs a turn while doing backward line following.
 * \return Returns TRUE while turn is still in progress.
 */
uint8_t Maze_EvaluateTurnBw(void) {
  REF_LineKind historyLineKind, currLineKind;
  TURN_Kind turn;

  HISTORY_Clear(); /* clear values */
  HISTORY_SampleSensors(); /* store current values */
  TURN_Turn(TURN_STEP_LINE_BW, HISTORY_SampleTurnStopFunction); /* make step over line */
  historyLineKind = HISTORY_LineKind(); /* new read new values */
  currLineKind = REF_GetLineKind(REF_LINE_KIND_MODE_MAZE);
#if MAZE_DEBUG_PRINT
  SHELL_SendString((unsigned char*)" history: ");
  SHELL_SendString((unsigned char*)REF_LineKindStr(historyLineKind));
  SHELL_SendString((unsigned char*)" curr: ");
  SHELL_SendString((unsigned char*)REF_LineKindStr(currLineKind));
  SHELL_SendString((unsigned char*)"\r\n");
#endif
  turn = Maze_SelectTurnBw(historyLineKind, currLineKind);
  if (turn==TURN_STOP) { /* should not happen here? */
    LineFollow_StopFollowing();
    McuLog_info("stopped");
    return ERR_FAILED; /* error case */
  } else { /* turn or do something */
#if MAZE_DEBUG_PRINT
    SHELL_SendString((unsigned char*)"bw turning ");
    SHELL_SendString((unsigned char*)TURN_TurnKindStr(turn));
    SHELL_SendString((unsigned char*)"\r\n");
#endif
    TURN_Turn(TURN_STEP_LINE_FW_AND_PAST_LINE, NULL); /* step over intersection */ /* ???? not good for going backward over -| */
    TURN_Turn(turn, NULL); /* make turn */
    Maze_AddPath(MirrorTurn(turn));
    Maze_SimplifyPath();
    return ERR_OK; /* turn finished */
  }
}

/*!
 * \brief Performs a turn.
 * \return Returns TRUE while turn is still in progress.
 */
uint8_t Maze_EvaluteTurn(bool *finished, bool *deadEndGoBw) {
  REF_LineKind historyLineKind, currLineKind;
  TURN_Kind turn;

  *finished = FALSE; /* defaults */
  *deadEndGoBw = FALSE; /* default */
  currLineKind = REF_GetLineKind(REF_LINE_KIND_MODE_MAZE);
  if (currLineKind==REF_LINE_NONE) { /* nothing, must be dead end */
    McuLog_trace("Maze, found dead end");
#if PL_GO_DEADEND_BW
    TURN_Turn(TURN_STEP_LINE_BW, NULL); /* step back so we are again on the line for line following */
    turn = TURN_STRAIGHT;
    *deadEndGoBw = TRUE;
#else
    turn = TURN_LEFT180;
#endif
  } else {
    McuLog_trace("Maze, determine road type");
    HISTORY_Clear(); /* clear history values */
    HISTORY_SampleSensors(); /* store current values */
#if PL_CONFIG_USE_QUADRATURE
    TURN_Turn(TURN_STEP_LINE_FW_AND_PAST_LINE, HISTORY_SampleTurnStopFunction); /* do the line and beyond in one step */
#else
    TURN_Turn(TURN_STEP_LINE_FW, NULL); /* make forward step over line */
#endif
#if MAZE_DO_DEBUG_WAIT /* debug mode */
    DebugStopAndWait();
#endif
    historyLineKind = HISTORY_LineKind(); /* new read new values */
    currLineKind = REF_GetLineKind(REF_LINE_KIND_MODE_MAZE_LINE_FOLLOW);
    turn = Maze_SelectTurn(historyLineKind, currLineKind);
    McuLog_trace("past: %s, curr: %s, turn %s", REF_LineKindStr(historyLineKind), REF_LineKindStr(currLineKind), TURN_TurnKindStr(turn));
  }
#if MAZE_DO_DEBUG_WAIT /* debug mode */
  DebugStopAndWait();
#endif
  if (turn==TURN_FINISH) {
    *finished = TRUE;
    McuLog_trace("MAZE: finish area!");
#if PL_CONFIG_USE_BUZZER
    Buzzer_PlayTune(BUZ_TUNE_MAZE_DESTINATION);
#endif
    return ERR_OK;
  } else if (turn==TURN_STRAIGHT && *deadEndGoBw) {
    Maze_AddPath(TURN_LEFT180); /* would have been a turn around */
    Maze_SimplifyPath();
    McuLog_trace("MAZE: going backward");
    return ERR_OK;
  } else if (turn==TURN_STRAIGHT) {
    Maze_AddPath(turn);
    Maze_SimplifyPath();
    McuLog_trace("MAZE: going straight");
    return ERR_OK;
  } else if (turn==TURN_STOP) { /* should not happen here? */
    LineFollow_StopFollowing();
    McuLog_trace("MAZE: Failure, stopped!!!");
    return ERR_FAILED; /* error case */
  } else { /* turn or do something */
#if MAZE_DEBUG_PRINT
    McuLog_trace("turning %s", TURN_TurnKindStr(turn));
#endif
#if !PL_CONFIG_USE_QUADRATURE /* if using quadrature, we stepped this piece already above */
    if (turn==TURN_LEFT90 || turn==TURN_RIGHT90) {
      TURN_Turn(TURN_STEP_PAST_LINE_FW, NULL); /* step before doing the turn so we turn on the middle of the intersection */
    }
#endif
    TURN_Turn(turn, NULL); /* make turn */
    Maze_AddPath(turn);
    Maze_SimplifyPath();
    return ERR_OK; /* turn finished */
  }
}

static TURN_Kind RevertTurn(TURN_Kind turn) {
  if (turn==TURN_LEFT90) {
    turn = TURN_RIGHT90;
  } else if (turn==TURN_RIGHT90) {
    turn = TURN_LEFT90;
  }
  return turn;
}

/**
 * \brief Reverts the path 
 */
static void Maze_RevertPath(void) {
  int i, j;
  TURN_Kind tmp;
  
  if (pathLength==0) {
    return;
  }
  j = pathLength-1;
  i = 0;
  while(i<=j) {
    tmp = path[i];
    path[i] = RevertTurn(path[j]);
    path[j] = RevertTurn(tmp);
    i++; j--;
  }
}

TURN_Kind Maze_SelectTurn(REF_LineKind prev, REF_LineKind curr) {
  if (prev==REF_LINE_NONE && curr==REF_LINE_NONE) { /* dead end */
    if (Maze_IsLeftHandRule()) {
      return TURN_RIGHT180; /* make U turn */
    } else {
      return TURN_LEFT180; /* make U turn */
    }
  } else if (prev==REF_LINE_FULL && curr==REF_LINE_NONE) { /* 'T' */
    if (Maze_IsLeftHandRule()) {
      return TURN_LEFT90;
    } else {
      return TURN_RIGHT90;
    }
  } else if (prev==REF_LINE_RIGHT && curr==REF_LINE_NONE) { /* right turn */
    return TURN_RIGHT90;
  } else if (prev==REF_LINE_LEFT && curr==REF_LINE_NONE) { /* left turn */
    return TURN_LEFT90;
  } else if (prev==REF_LINE_FULL && curr==REF_LINE_STRAIGHT) { /* '+' intersection */
    if (Maze_IsLeftHandRule()) {
      return TURN_LEFT90;
    } else {
      return TURN_RIGHT90;
    }
  } else if (prev==REF_LINE_FULL && curr==REF_LINE_FULL) { /* finish area */
    return TURN_FINISH;
  } else if (prev==REF_LINE_RIGHT && curr==REF_LINE_STRAIGHT) { /* forward and right turn */
    if (Maze_IsLeftHandRule()) {
      return TURN_STRAIGHT;
    } else {
      return TURN_RIGHT90;
    }
  } else if (prev==REF_LINE_LEFT && curr==REF_LINE_STRAIGHT) { /* forward and left turn */
    if (Maze_IsLeftHandRule()) {
      return TURN_LEFT90;
    } else {
      return TURN_STRAIGHT;
    }
  }
  return TURN_STOP; /* error case */
}

TURN_Kind Maze_SelectTurnBw(REF_LineKind prev, REF_LineKind curr) {
  if (prev==REF_LINE_LEFT && curr==REF_LINE_NONE) { /* was turn to left */
    return TURN_LEFT90;
  } else if (prev==REF_LINE_RIGHT && curr==REF_LINE_NONE) { /* was turn to right */
    return TURN_RIGHT90;
  } else if (prev==REF_LINE_RIGHT && curr==REF_LINE_STRAIGHT) { /* |- */
    if (Maze_IsLeftHandRule()) {
      return TURN_RIGHT90;
    } else {
      return TURN_LEFT180;
    }
  } else if (prev==REF_LINE_LEFT && curr==REF_LINE_STRAIGHT) { /* -| */
    if (Maze_IsLeftHandRule()) {
      return TURN_RIGHT180;
    } else {
      return TURN_LEFT90;
    }
  } else if (prev==REF_LINE_FULL && curr==REF_LINE_NONE) { /* T upside-down */
    if (Maze_IsLeftHandRule()) {
      return TURN_RIGHT90;
    } else {
      return TURN_LEFT90;
    }
  } else if (prev==REF_LINE_FULL && curr==REF_LINE_STRAIGHT) { /* '+' intersection  */
    if (Maze_IsLeftHandRule()) {
      return TURN_RIGHT90;
    } else {
      return TURN_LEFT90;
    }
  }
  return TURN_STOP; /* error case */
}

void Maze_SetSolved(void) {
  isSolved = TRUE;
  Maze_RevertPath();
  Maze_AddPath(TURN_STOP); /* add an action to stop */
}

bool Maze_IsSolved(void) {
  return isSolved;
}

void Maze_AddPath(TURN_Kind kind) {
  if (pathLength<MAZE_MAX_PATH) {
    path[pathLength] = kind;
    pathLength++;
  } else {
    /* error! */
  }
}

/*!
 * \brief Performs path simplification.
 * The idea is that whenever we encounter x-TURN_RIGHT180-x or x-TURN_LEFT180-x, we simplify it by cutting the dead end.
 * For example if we have TURN_LEFT90-TURN_RIGHT180-TURN_LEFT90, this can be simplified with TURN_STRAIGHT.
 */
void Maze_SimplifyPath(void) {
  /* implemented for left-hand-on-the-wall rule for now! */
  uint16_t totalAngle, i;
  
  if (pathLength<3) { /* we need at least 3 entries for simplification */
    return;
  }
  if (!(path[pathLength-2] == TURN_RIGHT180 || path[pathLength-2] == TURN_LEFT180)) {
    /* we simplify only if second-to-last is a dead-end */
    return;
  }
  totalAngle = 0;
  for (i=1; i<=3; i++) {
    switch(path[pathLength-i]) {
      case TURN_LEFT90: totalAngle += 270; break;
      case TURN_RIGHT90: totalAngle += 90; break;
      case TURN_LEFT180: totalAngle += 180; break;
      case TURN_RIGHT180: totalAngle += 180; break;
      default:
        /* would be an error? */
        break;
    } /* switch */
  }
  totalAngle %= 360; /* make sure angle is within 0...360 degrees */
  switch(totalAngle) {
    case 0:   path[pathLength-3] = TURN_STRAIGHT; break;
    case 90:  path[pathLength-3] = TURN_RIGHT90; break;
    case 180: path[pathLength-3] = TURN_LEFT180; break;
    case 270: path[pathLength-3] = TURN_LEFT90; break;
  }
  pathLength -= 2; /* was able to cut the path by 2 entries :-) */
}

static void Maze_PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"maze", (unsigned char*)"Group of maze following commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows maze help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  clear", (unsigned char*)"Clear the maze solution\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  rule (left|right)", (unsigned char*)"Left or Right-hand-on-the-wall rule\r\n", io->stdOut);
}

static void Maze_PrintStatus(const McuShell_StdIOType *io) {
  int i;
  
  McuShell_SendStatusStr((unsigned char*)"maze", (unsigned char*)"maze solving status\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  rule", Maze_IsLeftHandRule()?(unsigned char*)"left-hand-on-the-wall\r\n":(unsigned char*)"right-hand-on-the-wall\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  solved", Maze_IsSolved()?(unsigned char*)"yes\r\n":(unsigned char*)"no\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  path", (unsigned char*)"(", io->stdOut);
  McuShell_SendNum8u(pathLength, io->stdOut);
  McuShell_SendStr((unsigned char*)") ", io->stdOut);
  for(i=0;i<pathLength;i++) {
    McuShell_SendStr(TURN_TurnKindStr(path[i]), io->stdOut);
    McuShell_SendStr((unsigned char*)" ", io->stdOut);
  }
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
}

uint8_t Maze_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  uint8_t res = ERR_OK;

  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"maze help")==0) {
    Maze_PrintHelp(io);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"maze status")==0) {
    Maze_PrintStatus(io);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"maze clear")==0) {
    Maze_ClearSolution();
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"maze rule left")==0) {
    useLeftHandOnTheWallRule = TRUE;
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"maze rule right")==0) {
    useLeftHandOnTheWallRule = FALSE;
    *handled = TRUE;
  }
  return res;
}

TURN_Kind Maze_GetSolvedTurn(uint8_t *solvedIdx) {
  if (*solvedIdx < pathLength) {
    return path[(*solvedIdx)++];
  } else {
    return TURN_STOP; 
  }
}

void Maze_ClearSolution(void) {
  isSolved = FALSE;
  pathLength = 0;
}

#if 0
typedef enum MazeStateType_e {
  Maze_STATE_INIT,
  Maze_STATE_IDLE,
  Maze_STATE_FOLLOW_LINE
} MazeStateType_e;

static MazeStateType_e mazeState = Maze_STATE_INIT;

static void Maze_StateMachine(void) {
  switch (mazeState) {
    case Maze_STATE_INIT:
#if PL_CONFIG_USE_LEDS
      McuLED_On(LEDS_Left);
      McuLED_On(LEDS_Right);
#endif
      if (REF_CanUseSensor()) {
        mazeState = Maze_STATE_IDLE;
      }
      break;
    case Maze_STATE_IDLE:
      if (!REF_CanUseSensor()) { /* sensor not available any more? */
        mazeState = Maze_STATE_INIT;
      }
#if PL_CONFIG_USE_LEDS
      McuLED_Off(LEDS_Left);
      McuLED_On(LEDS_Right);
#endif
      if (EVNT_EventIsSet(EVNT_Maze_BUTTON_PRESS)) {
        EVNT_ClearEvent(EVNT_Maze_BUTTON_PRESS);
        LF_StartFollowing();
        mazeState = Maze_STATE_FOLLOW_LINE;
      }
      break;
    case Maze_STATE_FOLLOW_LINE:
#if PL_CONFIG_USE_LEDS
      McuLED_Off(LEDS_Left);
      McuLED_Off(LEDS_Right);
#endif
      if (!LF_IsFollowing()) {
        mazeState = Maze_STATE_IDLE;
      }
      if (EVNT_EventIsSet(EVNT_Maze_BUTTON_PRESS)) {
        EVNT_ClearEvent(EVNT_Maze_BUTTON_PRESS);
        LF_StopFollowing(); 
        mazeState = Maze_STATE_IDLE;
      }
      break;
  } /* switch */
}
#endif

void Maze_Init(void) {
  #if 0
  const NVMC_RobotData *ptr;

  ptr = NVMC_GetRobotData();
  if (ptr!=NULL) {
    (void)Maze_SetHandRule(ptr->MazeLeftHandOnTheWall);
  }
  #else
    Maze_SetHandRule(true);
  #endif
  Maze_ClearSolution();
}
#endif /* PL_CONFIG_MAZE_SOLVING */
