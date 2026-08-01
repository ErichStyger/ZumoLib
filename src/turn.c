/**
 * \file
 * \brief Robot turning interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * This module implements turning of the robot.
 */

#include "platform.h"
#if PL_CONFIG_USE_TURN
#include "turn.h"
#include "motor.h"
#include "McuWait.h"
#include "McuUtility.h"
#include "McuLog.h"
#if PL_CONFIG_USE_SHELL
  #include "McuShell.h"
  #include "shell.h"
#endif
#include "quadCounter.h"
#include "pid.h"
#if PL_CONFIG_USE_DRIVE
  #include "drive.h"
#endif

static int32_t TURN_Steps90 = TURN_STEPS_90;
static int32_t TURN_StepsLine = TURN_STEPS_LINE;
static int32_t TURN_StepsPastLine = TURN_STEPS_PAST_LINE;

void TURN_SetSteps(int32_t steps90, int32_t stepsLine, int32_t stepsPastLine) {
  TURN_StepsLine = stepsLine;
  TURN_StepsPastLine = stepsPastLine;
  TURN_Steps90 = steps90;
}

/*!
 * \brief Translate a turn kind into a string
 * \return Returns a descriptive string
 */
const unsigned char *TURN_TurnKindStr(TURN_Kind kind) {
  switch(kind) {
    case TURN_LEFT45:                   return (const unsigned char*)"LEFT45";
    case TURN_LEFT90:                   return (const unsigned char*)"LEFT90";
    case TURN_RIGHT45:                  return (const unsigned char*)"RIGHT45";
    case TURN_RIGHT90:                  return (const unsigned char*)"RIGHT90";
    case TURN_LEFT180:                  return (const unsigned char*)"LEFT180";
    case TURN_RIGHT180:                 return (const unsigned char*)"RIGHT180";
    case TURN_STRAIGHT:                 return (const unsigned char*)"STRAIGHT";
    case TURN_STEP_LINE_FW:             return (const unsigned char*)"STEP_LINE_FW";
    case TURN_STEP_LINE_BW:             return (const unsigned char*)"STEP_LINE_BW";
    case TURN_STEP_PAST_LINE_FW:        return (const unsigned char*)"STEP_PAST_LINE_FW";
    case TURN_STEP_PAST_LINE_BW:        return (const unsigned char*)"STEP_PAST_LINE_BW";
    case TURN_STEP_LINE_FW_AND_PAST_LINE: return (const unsigned char*)"STEP_LINE_FW_PAST_LINE";
    case TURN_STEP_LINE_BW_AND_PAST_LINE: return (const unsigned char*)"STEP_LINE_BW_PAST_LINE";
    case TURN_STOP:                     return (const unsigned char*)"STOP";
    case TURN_FINISH:                   return (const unsigned char*)"FINISH";
    default:                            return (const unsigned char*)"TURN_UNKNOWN!";
  }
}

void TURN_MoveToPos(int32_t targetLPos, int32_t targetRPos, bool wait, TURN_StopFct stopIt, int32_t timeoutMs) {
#if PL_CONFIG_USE_DRIVE
  (void)DRV_SetPos(targetLPos, targetRPos);
  (void)DRV_SetMode(DRV_MODE_POS);
#else
  (void)targetLPos;
  (void)targetRPos;
  #warning "not possible to move to a position without drive enabled!"
#endif
  for(;;) { /* breaks */
    if (stopIt!=NULL) {
      if (stopIt()) { /* check stop condition */
        break;
      }
    }
    McuWait_WaitOSms(1); /* wait some time, and give Drive module enough time to use new value */
    timeoutMs--;
    if (timeoutMs<=0) {
      break; /* timeout */
    }
#if PL_CONFIG_USE_DRIVE
    if (wait && DRV_HasTurned()) {
      break;
    }
#endif
    if (!wait) {
      break;
    }
  } /* for */
#if PL_CONFIG_USE_SHELL
  if (timeoutMs<=0) {
    McuLog_trace("MoveToPos Timeout");
  }
#endif
  (void)DRV_SetMode(DRV_MODE_STOP); /* stop it for easier debugging */
}

static void StepsTurn(int32_t stepsL, int32_t stepsR, TURN_StopFct stopIt, int32_t timeOutMS) {
  int32_t currLPos, currRPos, targetLPos, targetRPos;
  currLPos = QuadCounter_GetPosLeft();
  currRPos = QuadCounter_GetPosRight();
  targetLPos = currLPos+stepsL;
  targetRPos = currRPos+stepsR;
  TURN_MoveToPos(targetLPos, targetRPos, TRUE, stopIt, timeOutMS); /* go to final position */
}

static void PastTurn(void) {
#if TURN_WAIT_AFTER_STEP_MS > 0
#if PL_CONFIG_USE_DRIVE
  DRV_SetMode(DRV_MODE_NONE); /* disable any drive mode */
#endif
  McuWait_WaitOSms(TURN_WAIT_AFTER_STEP_MS);
#endif /* TURN_WAIT_AFTER_STEP_MS */
}

void TURN_Turn(TURN_Kind kind, TURN_StopFct stopIt) {
  switch(kind) {
    case TURN_LEFT45:
      StepsTurn(-TURN_Steps90/2, TURN_Steps90/2, stopIt, TURN_STEPS_90_TIMEOUT_MS/2);
      break;
    case TURN_RIGHT45:
      StepsTurn(TURN_Steps90/2, -TURN_Steps90/2, stopIt, TURN_STEPS_90_TIMEOUT_MS/2);
      break;
    case TURN_LEFT90:
      StepsTurn(-TURN_Steps90, TURN_Steps90, stopIt, TURN_STEPS_90_TIMEOUT_MS);
      break;
    case TURN_RIGHT90:
      StepsTurn(TURN_Steps90, -TURN_Steps90, stopIt, TURN_STEPS_90_TIMEOUT_MS);
      break;
    case TURN_LEFT180:
      StepsTurn(-(2*TURN_Steps90), 2*TURN_Steps90, stopIt, TURN_STEPS_90_TIMEOUT_MS*2);
     break;
    case TURN_RIGHT180:
      StepsTurn(2*TURN_Steps90, -(2*TURN_Steps90), stopIt, TURN_STEPS_90_TIMEOUT_MS*2);
     break;
    case TURN_STEP_BORDER_BW:
      StepsTurn(-(3*TURN_StepsLine), -(3*TURN_StepsLine), stopIt, TURN_STEPS_LINE_TIMEOUT_MS);
      break;
    case TURN_STEP_LINE_FW:
      StepsTurn(TURN_StepsLine, TURN_StepsLine, stopIt, TURN_STEPS_LINE_TIMEOUT_MS);
      break;
    case TURN_STEP_LINE_BW:
      StepsTurn(-TURN_StepsLine, -TURN_StepsLine, stopIt, TURN_STEPS_LINE_TIMEOUT_MS);
      break;
    case TURN_STEP_PAST_LINE_FW:
      StepsTurn(TURN_StepsPastLine, TURN_StepsPastLine, stopIt, TURN_STEPS_PAST_LINE_TIMEOUT_MS);
      break;
    case TURN_STEP_PAST_LINE_BW:
      StepsTurn(-TURN_StepsPastLine, -TURN_StepsPastLine, stopIt, TURN_STEPS_PAST_LINE_TIMEOUT_MS);
      break;
    case TURN_STEP_LINE_FW_AND_PAST_LINE: /* combination of TURN_STEP_LINE_FW and TURN_STEP_PAST_LINE_FW */
      StepsTurn(TURN_StepsLine+TURN_StepsPastLine, TURN_StepsLine+TURN_StepsPastLine, stopIt, TURN_STEPS_LINE_TIMEOUT_MS+TURN_STEPS_PAST_LINE_TIMEOUT_MS);
      break;
    case TURN_STEP_LINE_BW_AND_PAST_LINE: /* combination of TURN_STEP_LINE_BW and TURN_STEP_PAST_LINE_BW */
      StepsTurn(-TURN_StepsLine-TURN_StepsPastLine, -TURN_StepsLine-TURN_StepsPastLine, stopIt, TURN_STEPS_LINE_TIMEOUT_MS+TURN_STEPS_PAST_LINE_TIMEOUT_MS);
      break;
    case TURN_STOP_LEFT:
      Motor_SetSpeedPercent(Motor_GetMotorHandle(MOTOR_SIDE_LEFT), 0);
      break;
    case TURN_STOP_RIGHT:
      Motor_SetSpeedPercent(Motor_GetMotorHandle(MOTOR_SIDE_RIGHT), 0);
      break;
    case TURN_STOP:
      Motor_SetSpeedPercent(Motor_GetMotorHandle(MOTOR_SIDE_LEFT), 0);
      Motor_SetSpeedPercent(Motor_GetMotorHandle(MOTOR_SIDE_RIGHT), 0);
      break;
  default:
    break;
  }
  /*lint -save  -e522 Highest operation, function 'PastTurn', lacks side-effect. */
  PastTurn(); /* perform past-turn action */
  /*lint -restore Highest operation, function 'PastTurn', lacks side-effect. */
}

void TURN_TurnAngle(int16_t angle, TURN_StopFct stopIt) {
  bool isLeft = angle<0;
  int32_t steps;
  
  if (isLeft) {
    angle = -angle; /* make it positive */
  }
  angle %= 360; /* keep it inside 360� */
  steps = (angle*TURN_Steps90)/90;
  if (isLeft) {
    StepsTurn(-steps, steps, stopIt, ((angle/90)+1)*TURN_STEPS_90_TIMEOUT_MS);
  } else { /* right */
    StepsTurn(steps, -steps, stopIt, ((angle/90)+1)*TURN_STEPS_90_TIMEOUT_MS);
  }
  /*lint -save  -e522 Highest operation, function 'PastTurn', lacks side-effect. */
  PastTurn(); /* perform past-turn action */
  /*lint -restore Highest operation, function 'PastTurn', lacks side-effect. */
}

#if PL_CONFIG_USE_SHELL
static void TURN_PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"turn", (unsigned char*)"Group of turning commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows turn help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  <angle>", (unsigned char*)"Turn the robot by angle, negative is counter-clockwise, e.g. 'turn -90'\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  forward line", (unsigned char*)"Move forward over line\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  forward pastline", (unsigned char*)"Move forward past the line to center robot\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  backward", (unsigned char*)"Move one step backward\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  steps90 <steps>", (unsigned char*)"Number of steps for a 90 degree turn\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  stepsline <steps>", (unsigned char*)"Number of steps for stepping over a line\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  stepspastline <steps>", (unsigned char*)"Number of steps for a step after the line to center robot on intersection\r\n", io->stdOut);
}

static void TURN_PrintStatus(const McuShell_StdIOType *io) {
  unsigned char buf[32];

  McuShell_SendStatusStr((unsigned char*)"turn", (unsigned char*)"Turning status\r\n", io->stdOut);

  McuUtility_Num32sToStr(buf, sizeof(buf), TURN_Steps90);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" steps\r\n");
  McuShell_SendStatusStr((unsigned char*)"  90 degree", buf, io->stdOut);

  McuUtility_Num32sToStr(buf, sizeof(buf), TURN_StepsLine);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" steps\r\n");
  McuShell_SendStatusStr((unsigned char*)"  line", buf, io->stdOut);

  McuUtility_Num32sToStr(buf, sizeof(buf), TURN_StepsPastLine);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" steps\r\n");
  McuShell_SendStatusStr((unsigned char*)"  pastline", buf, io->stdOut);

  McuUtility_Num32sToStr(buf, sizeof(buf), QuadCounter_GetPosLeft());
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)", ");
  McuUtility_strcatNum16u(buf, sizeof(buf), QuadCounter_GetErrorsLeft());
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" errors\r\n");
  McuShell_SendStatusStr((unsigned char*)"  left pos", buf, io->stdOut);
  
  McuUtility_Num32sToStr(buf, sizeof(buf), QuadCounter_GetPosRight());
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)", ");
  McuUtility_strcatNum16u(buf, sizeof(buf), QuadCounter_GetErrorsRight());
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" errors\r\n");
  McuShell_SendStatusStr((unsigned char*)"  right pos", buf, io->stdOut);
}

static bool isNumberStart(uint8_t ch) {
  if (ch=='-') { /* negative number start */
    return TRUE;
  } else if (ch>='0' && ch<='9') {
    return TRUE;
  }
  return FALSE;
}

uint8_t TURN_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  uint8_t res = ERR_OK;
  const unsigned char *p;
  uint16_t val16u;

  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"turn help")==0) {
    TURN_PrintHelp(io);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"turn status")==0) {
    TURN_PrintStatus(io);
    *handled = TRUE;
  } else if (McuUtility_strncmp((char*)cmd, (char*)"turn ", sizeof("turn ")-1)==0 && isNumberStart(cmd[sizeof("turn ")-1])) {
    int32_t angle;

    p = cmd+sizeof("turn ")-1;
    if (McuUtility_xatoi(&p, &angle)==ERR_OK) {
      TURN_TurnAngle((int16_t)angle, NULL);
      TURN_Turn(TURN_STOP, NULL);
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument\r\n", io->stdErr);
      res = ERR_FAILED;
    }
  } else if (McuUtility_strcmp((char*)cmd, (char*)"turn forward pastline")==0) {
    TURN_Turn(TURN_STEP_LINE_FW_AND_PAST_LINE, NULL);
    TURN_Turn(TURN_STOP, NULL);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"turn forward line")==0) {
    TURN_Turn(TURN_STEP_LINE_FW, NULL);
    TURN_Turn(TURN_STOP, NULL);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"turn backward")==0) {
    TURN_Turn(TURN_STEP_LINE_BW, NULL);
    TURN_Turn(TURN_STOP, NULL);
    *handled = TRUE;
  } else if (McuUtility_strncmp((char*)cmd, (char*)"turn steps90 ", sizeof("turn steps90 ")-1)==0) {
    p = cmd+sizeof("turn steps90");
    if (McuUtility_ScanDecimal16uNumber(&p, &val16u)==ERR_OK) {
      TURN_Steps90 = val16u;
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument\r\n", io->stdErr);
      res = ERR_FAILED;
    }
  } else if (McuUtility_strncmp((char*)cmd, (char*)"turn stepspastline ", sizeof("turn stepspastline ")-1)==0) {
    p = cmd+sizeof("turn stepspastline");
    if (McuUtility_ScanDecimal16uNumber(&p, &val16u)==ERR_OK) {
      TURN_StepsPastLine = val16u;
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument\r\n", io->stdErr);
      res = ERR_FAILED;
    }
  } else if (McuUtility_strncmp((char*)cmd, (char*)"turn stepsline ", sizeof("turn stepsline ")-1)==0) {
    p = cmd+sizeof("turn stepsline");
    if (McuUtility_ScanDecimal16uNumber(&p, &val16u)==ERR_OK) {
      TURN_StepsLine = val16u;
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument\r\n", io->stdErr);
      res = ERR_FAILED;
    }
  }
  return res;
}
#endif /* PL_CONFIG_USE_SHELL */

void TURN_Init(void) {
  TURN_Steps90 = TURN_STEPS_90;
  TURN_StepsPastLine = TURN_STEPS_PAST_LINE;
  TURN_StepsLine = TURN_STEPS_LINE;
}
#endif /* PL_HAS_TURN */
