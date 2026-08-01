/**
 * \file
 * \brief Implements line following of the robot
 * \author Erich Styger, erich.styger@hslu.ch
* \license SPDX-License-Identifier: BSD-3-Clause
 * This is the implementation of the line following.
 */

#include "platform.h"
#if PL_CONFIG_LINE_FOLLOWING
#include "lineFollow.h"
#include "McuRTOS.h"
#include "McuShell.h"
#include "shell.h"
#include "motor.h"
#include "reflectance.h"
#include "McuWait.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "pid.h"
#include "turn.h"
#if PL_CONFIG_MAZE_SOLVING
  #include "maze.h"
#endif
#if PL_CONFIG_USE_BUZZER
  #include "buzzer.h"
#endif
#if PL_CONFIG_USE_DRIVE
  #include "drive.h"
#endif
#if PL_CONFIG_USE_LEDS
  #include "leds.h"
  #include "McuLED.h"
#endif

#define LINE_FOLLOW_FW      (1)   /* test setting to do forward line following */
#define LINE_DO_DEBUG_WAIT  (0)   /* if set to one, it will add some debug wait to stop the engines */
#if LINE_DO_DEBUG_WAIT
  #define LINE_DO_DEBUG_WAIT_MS  (500) /* waiting time in milliseconds */
#endif

#if LINE_DO_DEBUG_WAIT /* debug mode */
static void DebugStopAndWait(void) {
  static volatile bool waitHere = FALSE;

  TURN_Turn(TURN_STOP, NULL);
  vTaskDelay(LINE_DO_DEBUG_WAIT_MS);
  waitHere = true; /* set a breakpoint here */
}
#endif

#define PL_TURN_ON_FINISH  (0) /* temporary only */

typedef enum LINE_SpeedMode_e {
  LINE_SPEED_LOW,
  LINE_SPEED_MEDIUM,
  LINE_SPEED_HIGH
} LINE_SpeedMode_e;

static LINE_SpeedMode_e lineFollowSpeed = LINE_SPEED_LOW; /* set value in LineTask()! */
static bool LINE_isEnabled = true;
static TaskHandle_t lineTaskHandle = NULL;

typedef enum {
  STATE_IDLE,              /* idle, not doing anything */
  STATE_FOLLOW_SEGMENT,    /* line following segment, going forward */
#if PL_CONFIG_MAZE_SOLVING && PL_GO_DEADEND_BW
  STATE_FOLLOW_SEGMENT_BW, /* line following segment, going backward */
#endif
#if PL_CONFIG_MAZE_SOLVING
  STATE_TURN,              /* reached an intersection, turning around */
  STATE_FINISHED,          /* reached finish area */
#endif
  STATE_STOP               /* stop the engines */
} StateType;

/* task notification bits */
#define LineFollow_START_FOLLOWING  (1<<0)  /* start line following */
#define LineFollow_STOP_FOLLOWING   (1<<1)  /* stop line following */

static volatile StateType LineFollow_currState = STATE_IDLE;
#if PL_CONFIG_MAZE_SOLVING
static uint8_t LineFollow_solvedIdx = 0; /*  index to iterate through the solution, zero is the solution start index */
#endif

void REF_StartStopTrace(bool start);

void LineFollow_StartFollowing(void) {
  #if REFELECTANCE_CONFIG_DO_SENSOR_TRACING
  REF_StartStopRTTTrace(true);
  #endif
  (void)xTaskNotify(lineTaskHandle, LineFollow_START_FOLLOWING, eSetBits);
}

void LineFollow_StopFollowing(void) {
  (void)xTaskNotify(lineTaskHandle, LineFollow_STOP_FOLLOWING, eSetBits);
  #if REFELECTANCE_CONFIG_DO_SENSOR_TRACING
  REF_StartStopRTTTrace(false);
  #endif
}

void LineFollow_StartStopFollowing(void) {
  if (LineFollow_IsFollowing()) {
    LineFollow_StopFollowing();
  } else {
    LineFollow_StartFollowing();
  }
}

/*!
 * \brief follows a line segment.
 * \return Returns TRUE if still on line segment
 */
bool LineFollow_FollowSegment(REF_LineKindMode mode, bool forward) {
  uint16_t currLine;
  bool onLine;
  REF_LineKind currLineKind;

  currLineKind = REF_GetLineKind(mode);
#if PL_CONFIG_LINE_FOLLOWING
  if (currLineKind==REF_LINE_STRAIGHT || currLineKind!=REF_LINE_NONE) { /* as long we have something: follow line */
#elif PL_CONFIG_MAZE_SOLVING
  if (currLineKind==REF_LINE_STRAIGHT) { /* as long we have a line: follow line */
#endif
    currLine = REF_GetLineValue(&onLine);
    PID_Line(currLine, REF_MIDDLE_LINE_VALUE, REF_LineWidth(), forward); /* continue move along the line */
    return TRUE;
  } else { /* not a line any more */
    if (mode==REF_LINE_KIND_MODE_LINE_FOLLOW) { /* in line following mode, and not a line any more? stop the engines */
      LineFollow_StopFollowing(); /* send signal */
      DRV_SetMode(DRV_MODE_STOP);
      McuLog_trace("LineFollow_FollowSegment not a line: %s, stopped", REF_LineKindStr(currLineKind));
    }
    return FALSE; /* intersection/change of direction or not on line any more */
  }
}

/*!
 * \brief Move from outside onto a line/segment to follow it.
 * \return Returns TRUE if still on line segment
 */
bool LineFollow_MoveOnSegment(bool turningLeft) {
  uint16_t currLine, targetLine;
  int tmp;
  bool onLine;
  REF_LineKind currLineKind;
  uint16_t lineWidth;

  currLine = REF_GetLineValue(&onLine);
  currLineKind = REF_GetLineKind(REF_LINE_KIND_MODE_ALL);
  lineWidth = REF_LineWidth();
  if (lineWidth>2000) {
    if (turningLeft) {
      tmp = REF_MIDDLE_LINE_VALUE+lineWidth;
    } else {
      tmp = REF_MIDDLE_LINE_VALUE-lineWidth;
    }
    if (tmp<0) {
      tmp = 0;
    }
    if (tmp>REF_MAX_LINE_VALUE) {
      tmp = REF_MAX_LINE_VALUE;
    }
    targetLine = tmp;
  } else {
    targetLine = REF_MIDDLE_LINE_VALUE;
  }
  if (currLineKind==REF_LINE_STRAIGHT || currLineKind==REF_LINE_LEFT || currLineKind==REF_LINE_RIGHT || currLineKind==REF_LINE_FULL) {
    PID_Line(currLine, targetLine, REF_LineWidth(), TRUE); /* move along the line */
    return TRUE;
  } else {
    return FALSE; /* intersection/change of direction or not on line any more */
  }
}

bool LineFollow_FollowSegmentLinePos(REF_LineKindMode mode, uint16_t setLinePos) {
  uint16_t currLine;
  bool onLine;
  REF_LineKind currLineKind;

  currLine = REF_GetLineValue(&onLine);
  currLineKind = REF_GetLineKind(mode);
  if (currLineKind==REF_LINE_STRAIGHT) {
    PID_Line(currLine, setLinePos, REF_LineWidth(), TRUE); /* move along the line */
    return TRUE;
  } else {
    return FALSE; /* intersection/change of direction or not on line any more */
  }
}

static void StateMachine(void) {
  for(;;) {
    switch (LineFollow_currState) {
      case STATE_IDLE:
        break;
      case STATE_FOLLOW_SEGMENT:
#if PL_CONFIG_MAZE_SOLVING
        if (!LineFollow_FollowSegment(REF_LINE_KIND_MODE_MAZE, LINE_FOLLOW_FW)) {
          DRV_SetMode(DRV_MODE_STOP);
          McuLog_trace("Maze, and not line any more");
      #if LINE_DO_DEBUG_WAIT /* debug mode */
          DebugStopAndWait();
      #endif
          LineFollow_currState = STATE_TURN; /* make turn */
          continue; /* process next step immediately */
        }
#else
        {
          if (!LineFollow_FollowSegment(REF_LINE_KIND_MODE_LINE_FOLLOW, LINE_FOLLOW_FW)) {
            McuLog_trace("No segment any more! Line %s, stop!", REF_LineKindStr(REF_GetLineKind(REF_LINE_KIND_MODE_LINE_FOLLOW)));
            LineFollow_currState = STATE_STOP; /* stop if we do not have a line any more */
          }
        }
#endif
        break;

  #if PL_CONFIG_MAZE_SOLVING
      case STATE_TURN:
        if (Maze_IsSolved()) {
          TURN_Kind turn;

          turn = Maze_GetSolvedTurn(&LineFollow_solvedIdx);
          if (turn==TURN_STOP) { /* last turn reached */
            TURN_Turn(turn, NULL);
          #if PL_CONFIG_USE_DRIVE
            (void)DRV_SetMode(DRV_MODE_NONE); /* disable any drive mode */
          #endif
            LineFollow_currState = STATE_STOP;
            McuLog_trace("MAZE: I'm back to the start of maze!");
          #if PL_CONFIG_USE_BUZZER
            Buzzer_PlayTune(1);
          #endif
            Maze_ClearSolution(); /* clear solution */
  #if PL_TURN_ON_START
            /* turn the robot on start so it is ready to run the maze again */
          #if PL_CONFIG_USE_QUADRATURE
            TURN_Turn(TURN_LEFT180, NULL);
          #else /* not accurate enough without position sensor */
            TURN_Turn(TURN_LEFT90, NULL); /* do turn in two steps */
            TURN_Turn(TURN_LEFT90, NULL);
          #endif
  #endif
            TURN_Turn(TURN_STOP, NULL);
  #if PL_CONFIG_USE_DRIVE
            (void)DRV_SetMode(DRV_MODE_NONE); /* disable any drive mode */
  #endif
            /* now ready to solve maze again */
            McuLog_trace("MAZE: ready to start again!");
          } else { /* perform turning */
            TURN_Turn(TURN_STEP_LINE_FW_AND_PAST_LINE, NULL); /* Step over line */
            TURN_Turn(turn, NULL);
        #if LINE_DO_DEBUG_WAIT /* debug mode */
            DebugStopAndWait();
        #endif
        #if PL_CONFIG_USE_DRIVE
            (void)DRV_SetMode(DRV_MODE_NONE); /* disable any drive mode, so we can do line following (line following is PWM) */
        #endif
            LineFollow_currState = STATE_FOLLOW_SEGMENT;
            continue; /* process next state immediately */
          }
        } else { /* still evaluating maze */
          bool deadEndGoBw = FALSE;
          bool finished = FALSE;

          if (Maze_EvaluteTurn(&finished, &deadEndGoBw)==ERR_OK) { /* find out what intersection it is and make a turn */
          #if LINE_DO_DEBUG_WAIT /* debug mode */
            DebugStopAndWait();
          #endif
          #if PL_CONFIG_USE_DRIVE
            (void)DRV_SetMode(DRV_MODE_NONE); /* disable any drive mode */
          #endif
            if (finished) {
              LineFollow_currState = STATE_FINISHED;
              Maze_SetSolved();
              LineFollow_solvedIdx = 0; /* set index to start of solution */
            #if PL_CONFIG_USE_BUZZER
              Buzzer_PlayTune(BUZ_TUNE_MAZE_DESTINATION);
            #endif
            #if PL_TURN_ON_FINISH
              /* turn the robot */
              #if PL_CONFIG_USE_QUADRATURE
              TURN_Turn(TURN_LEFT180, NULL);
              #else /* not accurate enough without position sensor */
              TURN_Turn(TURN_LEFT90, NULL); /* do turn in two steps */
              TURN_Turn(TURN_LEFT90, NULL);
              #endif
            #endif
              //TURN_Turn(TURN_STOP, NULL);
          #if LINE_DO_DEBUG_WAIT /* debug mode */
              DebugStopAndWait();
          #endif
          #if PL_CONFIG_USE_DRIVE
              (void)DRV_SetMode(DRV_MODE_NONE); /* disable any drive mode */
          #endif
              /* now ready to do line following */
          #if PL_GO_DEADEND_BW
            } else if (deadEndGoBw) {
              LineFollow_currState = STATE_FOLLOW_SEGMENT_BW;
          #endif
            } else {
              LineFollow_currState = STATE_FOLLOW_SEGMENT;
            }
          } else { /* error case */
            LineFollow_currState = STATE_STOP;
          }
        }
        break;
  #endif
  #if PL_CONFIG_MAZE_SOLVING
      case STATE_FINISHED:
        McuLog_trace("MAZE: Found maze destination!");
    #if PL_CONFIG_USE_BUZZER
        Buzzer_PlayTune(BUZ_TUNE_MAZE_DESTINATION);
    #endif
        McuLog_trace("MAZE: Going back to start...");
        LineFollow_currState = STATE_FOLLOW_SEGMENT; /* go back to start */
        LineFollow_currState = STATE_STOP;
        break;
  #endif
      case STATE_STOP:
        McuLog_trace("Stopped!");
        DRV_SetMode(DRV_MODE_STOP);
        TURN_Turn(TURN_STOP, NULL);
        LineFollow_currState = STATE_IDLE;
        break;
    } /* switch */
    break; /* get out of for loop */
  } /* for */
}

bool LineFollow_IsFollowing(void) {
  return LineFollow_currState!=STATE_IDLE;
}

static const unsigned char *LINE_GetSpeedModeString(LINE_SpeedMode_e mode) {
  switch(mode) {
  case LINE_SPEED_LOW: return (const unsigned char*)"LOW";
  case LINE_SPEED_MEDIUM: return (const unsigned char*)"MEDIUM";
  case LINE_SPEED_HIGH: return (const unsigned char*)"HIGH";
  default: break;
  }
  return (const unsigned char*)"UNKNOWN";
}

static uint8_t LINE_SetSpeed(LINE_SpeedMode_e speed) {
  uint8_t res;
  PID_Config *lineFwPid, *posLeftPid, *posRightPid;

  res = PID_GetPIDConfig(PID_CONFIG_LINE_FW, &lineFwPid);
  if (res!=ERR_OK || lineFwPid==NULL) {
    return ERR_FAILED;
  }
  res = PID_GetPIDConfig(PID_CONFIG_POS_LEFT, &posLeftPid);
  if (res!=ERR_OK || posLeftPid==NULL) {
    return ERR_FAILED;
  }
  res = PID_GetPIDConfig(PID_CONFIG_POS_RIGHT, &posRightPid);
  if (res!=ERR_OK || posRightPid==NULL) {
    return ERR_FAILED;
  }
  switch(speed) {
    case LINE_SPEED_LOW:
      lineFwPid->maxSpeedPercent = 25;
      lineFwPid->pFactor100 = 4000;
      lineFwPid->dFactor100 = 500;
      TURN_SetSteps(650, 170, 240);
      posLeftPid->maxSpeedPercent = 35;
      posLeftPid->pFactor100 = 2000;
      posRightPid->pFactor100 = posLeftPid->pFactor100;
      posRightPid->maxSpeedPercent = posLeftPid->maxSpeedPercent;
      McuLog_info("Changed parameters for slow speed");
      break;
    case LINE_SPEED_MEDIUM:
      lineFwPid->maxSpeedPercent = 30; /* 15 => 30 */
      lineFwPid->pFactor100 = 5500;
      lineFwPid->iAntiWindup = 100000;
      TURN_SetSteps(700, 190, 50);
      posLeftPid->maxSpeedPercent = 40;
      posLeftPid->pFactor100 = 2000;
      posRightPid->pFactor100 = posLeftPid->pFactor100;
      posRightPid->maxSpeedPercent = posLeftPid->maxSpeedPercent;
      McuLog_info("Changed parameters for medium speed");
      break;
    case LINE_SPEED_HIGH: /* to be verified */
      lineFwPid->maxSpeedPercent = 40;
      lineFwPid->pFactor100 = 2000;
      lineFwPid->iAntiWindup = 30000;
      TURN_SetSteps(700, 150, 80);
      posLeftPid->maxSpeedPercent = 100;
      posLeftPid->pFactor100 = 2000;
      posRightPid->pFactor100 = posLeftPid->pFactor100;
      posRightPid->maxSpeedPercent = posLeftPid->maxSpeedPercent;
      McuLog_info("Changed parameters for high speed");
      break;
    default:
      return ERR_FAILED;
  } /* switch */
  lineFollowSpeed = speed;
  return ERR_OK;
}

static void lineTask (void *pvParameters) {
  uint32_t notifcationValue;
  BaseType_t notified;

  (void)pvParameters; /* not used */
  (void)LINE_SetSpeed(LINE_SPEED_LOW);
  for(;;) {
    if (!LINE_isEnabled) {
      vTaskSuspend(NULL);
    }
    notified = xTaskNotifyWait(0UL, LineFollow_START_FOLLOWING|LineFollow_STOP_FOLLOWING, &notifcationValue, 1); /* check flags, need to wait for one tick */
    if (notified==pdTRUE) { /* received notification */
      if (notifcationValue&LineFollow_START_FOLLOWING) {
        if (REF_CanUseSensor()) {
          McuLog_info("start line following");
          PID_Start();
          LineFollow_currState = STATE_FOLLOW_SEGMENT;
      #if PL_CONFIG_USE_DRIVE
          (void)DRV_SetMode(DRV_MODE_NONE); /* disable any drive mode */
      #endif
        } else {
          McuLog_error("Sensors not ready!");
      #if PL_CONFIG_USE_BUZZER
          (void)Buzzer_Beep(500, 500);
      #endif
        }
      }
      if (notifcationValue&LineFollow_STOP_FOLLOWING) {
        LineFollow_currState = STATE_STOP;
        McuLog_info("stopped line following");
      }
    }
    StateMachine();
#if PL_CONFIG_MAZE_SOLVING
    if (Maze_IsLeftHandRule()) {
  #if PL_CONFIG_USE_LEDS /* blink left LED */
      McuLED_Toggle(LEDS_Left);
      McuLED_Off(LEDS_Right);
  #endif
    } else {
  #if PL_CONFIG_USE_LEDS /* blink right LED */
      McuLED_Toggle(LEDS_Right);
      McuLED_Off(LEDS_Left);
  #endif
    }
#endif
    if (LineFollow_IsFollowing()) {
      vTaskDelay(pdMS_TO_TICKS(5));
    } else { /* give back more time */
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }
}

static void LineFollow_PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"line", (unsigned char*)"Group of line following commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows line help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  enable|disable", (unsigned char*)"Enable or disable line task\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  speed <mode>", (unsigned char*)"Line following speed, either low, medium or high\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  start|stop", (unsigned char*)"Starts or stops line following\r\n", io->stdOut);
}

static void LineFollow_PrintStatus(const McuShell_StdIOType *io) {
  uint8_t buf[32];

  McuShell_SendStatusStr((unsigned char*)"line follow", (unsigned char*)"Line following status\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  enabled", LINE_isEnabled?(unsigned char*)"yes\r\n":(unsigned char*)"no\r\n", io->stdOut);
  switch (LineFollow_currState) {
    case STATE_IDLE: 
      McuShell_SendStatusStr((unsigned char*)"  state", (unsigned char*)"IDLE\r\n", io->stdOut);
      break;
    case STATE_FOLLOW_SEGMENT: 
      McuShell_SendStatusStr((unsigned char*)"  state", (unsigned char*)"FOLLOW_SEGMENT\r\n", io->stdOut);
      break;
    case STATE_STOP: 
      McuShell_SendStatusStr((unsigned char*)"  state", (unsigned char*)"STOP\r\n", io->stdOut);
      break;
#if PL_CONFIG_MAZE_SOLVING
    case STATE_TURN: 
      McuShell_SendStatusStr((unsigned char*)"  state", (unsigned char*)"TURN\r\n", io->stdOut);
      break;
    case STATE_FINISHED: 
      McuShell_SendStatusStr((unsigned char*)"  state", (unsigned char*)"FINISHED\r\n", io->stdOut);
      break;
#endif
    default: 
      McuShell_SendStatusStr((unsigned char*)"  state", (unsigned char*)"UNKNOWN\r\n", io->stdOut);
      break;
  } /* switch */
  McuUtility_strcpy(buf, sizeof(buf), LINE_GetSpeedModeString(lineFollowSpeed));
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  speed", buf, io->stdOut);
}

uint8_t LineFollow_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  uint8_t res = ERR_OK;

  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"line help")==0) {
    LineFollow_PrintHelp(io);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"line status")==0) {
    LineFollow_PrintStatus(io);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"line enable")==0) {
    *handled = TRUE;
    LINE_isEnabled = true;
    vTaskResume(lineTaskHandle);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"line disable")==0) {
    *handled = TRUE;
    LINE_isEnabled = false;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"line start")==0) {
    LineFollow_StartFollowing();
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"line stop")==0) {
    LineFollow_StopFollowing();
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"line speed low")==0) {
    (void)LINE_SetSpeed(LINE_SPEED_LOW);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"line speed medium")==0) {
    (void)LINE_SetSpeed(LINE_SPEED_MEDIUM);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"line speed high")==0) {
    (void)LINE_SetSpeed(LINE_SPEED_HIGH);
    *handled = TRUE;
  }
  return res;
}

void LineFollow_Deinit(void) {
  /* nothing needed */
}

void LineFollow_Init(void) {
  LineFollow_currState = STATE_IDLE;
  if (xTaskCreate(lineTask, "Line", (3*1024)/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+2, &lineTaskHandle) != pdPASS) {
    for(;;){} /* error */
  }
}
#endif /* PL_CONFIG_LINE_FOLLOWING */
