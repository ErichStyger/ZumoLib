/**
 * \file
 * \brief This is the implementation of the PID Module
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_PID
#include "pid.h"
#include "Motor.h"
#include "McuUtility.h"
#include "reflectance.h"

#define PID_DEBUG 0 /* careful: this will slow down the PID loop frequency! */

#if PL_CONFIG_USE_LINE_PID
  static PID_Config lineFwConfig;
#endif
#if PL_GO_DEADEND_BW
  static PID_Config lineBwConfig;
#endif
#if PL_CONFIG_USE_POS_PID
  static PID_Config posLeftConfig, posRightConfig;
#endif
#if PL_CONFIG_USE_SPEED_PID
  static PID_Config speedLeftConfig, speedRightConfig;
#endif

uint8_t PID_GetPIDConfig(PID_ConfigType config, PID_Config **confP) {
  switch(config) {
#if PL_CONFIG_USE_LINE_PID
    case PID_CONFIG_LINE_FW:
      *confP = &lineFwConfig; break;
#endif
#if PL_GO_DEADEND_BW
    case PID_CONFIG_LINE_BW:
      *confP = &lineBwConfig; break;
#endif
    case PID_CONFIG_POS_LEFT:
      *confP = &posLeftConfig; break;
    case PID_CONFIG_POS_RIGHT:
      *confP = &posRightConfig; break;
    case PID_CONFIG_SPEED_LEFT:
      *confP = &speedLeftConfig; break;
    case PID_CONFIG_SPEED_RIGHT:
      *confP = &speedRightConfig; break;
    default:
      *confP = NULL;
      return ERR_FAILED;
  }
  return ERR_OK;
}

#if PL_CONFIG_USE_SPEED_PID || PL_CONFIG_USE_POS_PID || PL_CONFIG_USE_LINE_PID
static int32_t PID(int32_t currVal, int32_t setVal, PID_Config *config) {
  int32_t error;
  int32_t pid;
  
  /* perform PID closed control loop calculation */
  error = setVal-currVal; /* calculate error */
  pid = (error*config->pFactor100)/100; /* P part */
  config->integral += error; /* integrate error */
  if (config->integral>config->iAntiWindup) {
    config->integral = config->iAntiWindup;
  } else if (config->integral<-config->iAntiWindup) {
    config->integral = -config->iAntiWindup;
  }
#if 1 /* see http://brettbeauregard.com/blog/2011/04/improving-the-beginner%E2%80%99s-pid-reset-windup/ */
  {
    int32_t max;

    max = 0xffff; /* max value of PWM */
    if (config->integral > max) {
      config->integral = max;
    } else if (config->integral < -max) {
      config->integral = -max;
    }
  }
#endif
  pid += (config->integral*config->iFactor100)/100; /* add I part */
  pid += ((error-config->lastError)*config->dFactor100)/100; /* add D part */
  config->lastError = error; /* remember for next iteration of D part */
  return pid;
}
#endif

#if PL_CONFIG_LINE_FOLLOWING || PL_CONFIG_MAZE_SOLVING
static int32_t Limit(int32_t val, int32_t minVal, int32_t maxVal) {
  if (val<minVal) {
    return minVal;
  } else if (val>maxVal) {
    return maxVal;
  }
  return val;
}

static Motor_Direction AbsSpeed(int32_t *speedP) {
  if (*speedP<0) {
    *speedP = -(*speedP);
    return MOTOR_DIR_BACKWARD;
  }
  return MOTOR_DIR_FORWARD;
}

#if PL_CONFIG_LINE_FOLLOWING || PL_CONFIG_MAZE_SOLVING
/*! \brief returns error (always positive) percent */
static uint8_t errorWithinPercent(int32_t error) {
  if (error<0) {
    error = -error;
  }
  error = error/(REF_MAX_LINE_VALUE/2/100);
  if (error>100) {
    error = 100;
  }
  return error;
}
#endif

#if PL_CONFIG_LINE_FOLLOWING || PL_CONFIG_MAZE_SOLVING
static void PID_LineCfg(uint16_t currLine, uint16_t setLine, uint16_t currLineWidth, bool forward, PID_Config *config) {
  int32_t pid, speed, speedL, speedR;
#if PID_DEBUG
  unsigned char buf[16];
  static uint8_t cnt = 0;
#endif
  uint8_t errorPercent;
  Motor_Direction directionL=MOTOR_DIR_FORWARD, directionR=MOTOR_DIR_FORWARD;
  
  (void)currLineWidth;

  pid = PID(currLine, setLine, config);
  errorPercent = errorWithinPercent(currLine-setLine);
  
  /* transform into different speed for motors. The PID is used as difference value to the motor PWM */
  if (!forward) { /* going backward */
    /* need to do it slow as sensor is on the 'back', and we cannot usual turns to get the sensor back 'on line' */
    speed = ((int32_t)config->maxSpeedPercent)*(0xffff/100)*6/10; /* %60 */
    pid = Limit(pid, -speed, speed);
    if (pid<0) { /* turn right */
      speedR = speed;
      speedL = speed-pid;
    } else { /* turn left */
      speedR = speed+pid;
      speedL = speed;
    }
#if 0 /* simple differential steering */
  } else {
    speed = ((int32_t)config->maxSpeedPercent)*(0xffff/100); /* 100% */
    pid = Limit(pid, -speed, speed);
    if (pid<0) { /* turn right */
      speedR = speed;
      speedL = speed-pid;
    } else { /* turn left */
      speedR = speed+pid;
      speedL = speed;
    }
  }
#else
  } else if (errorPercent <= 10) { /* pretty on center: move forward both motors with base speed */
    speed = ((int32_t)config->maxSpeedPercent)*(0xffff/100); /* 100% */
    pid = Limit(pid, -speed, speed);
    if (pid<0) { /* turn right */
      speedR = speed;
      speedL = speed-pid;
    } else { /* turn left */
      speedR = speed+pid;
      speedL = speed;
    }
  } else if (errorPercent <= 30) {
    /* outside left/right halve position from center, slow down one motor and speed up the other */
    speed = ((int32_t)config->maxSpeedPercent)*(0xffff/100)*10/10; /* 80% */
    pid = Limit(pid, -speed, speed);
    if (pid<0) { /* turn right */
      speedR = speed+pid; /* decrease speed */
      speedL = speed-pid; /* increase speed */
    } else { /* turn left */
      speedR = speed+pid; /* increase speed */
      speedL = speed-pid; /* decrease speed */
    }
  } else  {
    /* line is far to the left or right: use backward motor motion */
    speed = ((int32_t)config->maxSpeedPercent)*(0xffff/100)*10/10; /* %100 */
    if (pid<0) { /* turn right */
      speedR = -speed+pid; /* decrease speed */
      speedL = speed-pid; /* increase speed */
    } else { /* turn left */
      speedR = speed+pid; /* increase speed */
      speedL = -speed-pid; /* decrease speed */
    }
    speedL = Limit(speedL, -speed, speed);
    speedR = Limit(speedR, -speed, speed);
    directionL = AbsSpeed(&speedL);
    directionR = AbsSpeed(&speedR);
  }
#endif
  /* speed is now always positive, make sure it is within 16bit PWM boundary */
  if (speedL>0xFFFF) {
    speedL = 0xFFFF;
  } else if (speedL<0) {
    speedL = 0;
  }
  if (speedR>0xFFFF) {
    speedR = 0xFFFF;
  } else if (speedR<0) {
    speedR = 0;
  }
  if (!forward) { /* swap direction/speed */
    if (directionL==MOTOR_DIR_FORWARD) {
      directionL=MOTOR_DIR_BACKWARD;
    } else {
      directionL=MOTOR_DIR_FORWARD;
    }
    if (directionR==MOTOR_DIR_FORWARD) {
      directionR=MOTOR_DIR_BACKWARD;
    } else {
      directionR=MOTOR_DIR_FORWARD;
    }
  }
  /* send new speed values to motor */
  Motor_SetDirection(Motor_GetMotorHandle(MOTOR_SIDE_LEFT), directionL);
  Motor_SetDirection(Motor_GetMotorHandle(MOTOR_SIDE_RIGHT), directionR);
  Motor_SetVal(Motor_GetMotorHandle(MOTOR_SIDE_LEFT), 0xFFFF-speedL); /* PWM is low active */
  Motor_SetVal(Motor_GetMotorHandle(MOTOR_SIDE_RIGHT), 0xFFFF-speedR); /* PWM is low active */
#if PID_DEBUG /* debug diagnostic */
  {
    cnt++;
    if (cnt>10) { /* limit number of messages to the console */
      McuShell_StdIO_OutErr_FctType ioOut = SHELL_GetStdio()->stdOut;
      cnt = 0;
      
      McuShell_SendStr((unsigned char*)"line:", ioOut);
      buf[0] = '\0';
      McuUtility_strcatNum16u(buf, sizeof(buf), currLine);
      McuShell_SendStr(buf, ioOut);
  
      McuShell_SendStr((unsigned char*)" sum:", ioOut);
      buf[0] = '\0';
      McuUtility_strcatNum32Hex(buf, sizeof(buf), integral);
      McuShell_SendStr(buf, ioOut);
      
      McuShell_SendStr((unsigned char*)" left:", ioOut);
      McuShell_SendStr(directionL==MOTOR_DIR_FORWARD?(unsigned char*)"fw ":(unsigned char*)"bw ", ioOut);
      buf[0] = '\0';
      McuUtility_strcatNum16Hex(buf, sizeof(buf), speedL);
      McuShell_SendStr(buf, ioOut);
  
      McuShell_SendStr((unsigned char*)" right:", ioOut);
      McuShell_SendStr(directionR==MOTOR_DIR_FORWARD?(unsigned char*)"fw ":(unsigned char*)"bw ", ioOut);
      buf[0] = '\0';
      McuUtility_strcatNum16Hex(buf, sizeof(buf), speedR);
      McuShell_SendStr(buf, ioOut);
      
      McuShell_SendStr((unsigned char*)"\r\n", ioOut);
    }
  }
#endif
}
#endif //PL_CONFIG_LINE_FOLLOWING || PL_CONFIG_MAZE_SOLVING

#endif

#if 0
static void PID_WallMiddleCfg(int leftMM, int rightMM, PID_Config *config) {
  int32_t pid, speed, speedL, speedR;
  Motor_Direction directionL=MOTOR_DIR_FORWARD, directionR=MOTOR_DIR_FORWARD;
  
  pid = PID(leftMM, (leftMM+rightMM)/2, config);
  speed = ((int32_t)config->maxSpeedPercent)*(0xffff/100)*10/10; /* 80% */
  pid = Limit(pid, -speed, speed);
  if (pid>0) { /* turn right */
    speedL = speed+pid; /* decrease speed */
    speedR = speed-pid; /* increase speed */
  } else { /* turn left */
    speedL = speed+pid; /* increase speed */
    speedR = speed-pid; /* decrease speed */
  }
  /* speed is now always positive, make sure it is within 16bit PWM boundary */
  if (speedL>0xFFFF) {
    speedL = 0xFFFF;
  } else if (speedL<0) {
    speedL = 0;
  }
  if (speedR>0xFFFF) {
    speedR = 0xFFFF;
  } else if (speedR<0) {
    speedR = 0;
  }
  /* send new speed values to motor */
  Motor_SetDirection(Motor_GetMotorHandle(Motor_MOTOR_LEFT), directionL);
  Motor_SetDirection(Motor_GetMotorHandle(Motor_MOTOR_RIGHT), directionR);
  Motor_SetVal(Motor_GetMotorHandle(Motor_MOTOR_LEFT), 0xFFFF-speedL); /* PWM is low active */
  Motor_SetVal(Motor_GetMotorHandle(Motor_MOTOR_RIGHT), 0xFFFF-speedR); /* PWM is low active */
}
#endif
#if 0
static void PID_WallLeftCfg(int leftMM, int targetMM, PID_Config *config) {
  int32_t pid, speed, speedL, speedR;
  Motor_Direction directionL=MOTOR_DIR_FORWARD, directionR=MOTOR_DIR_FORWARD;
  
  pid = PID(leftMM, targetMM, config);
  speed = ((int32_t)config->maxSpeedPercent)*(0xffff/100)*10/10; /* 80% */
  pid = Limit(pid, -speed, speed);
  if (pid>0) { /* turn right */
    speedL = speed+pid; /* decrease speed */
    speedR = speed-pid; /* increase speed */
  } else { /* turn left */
    speedL = speed+pid; /* increase speed */
    speedR = speed-pid; /* decrease speed */
  }
  /* speed is now always positive, make sure it is within 16bit PWM boundary */
  if (speedL>0xFFFF) {
    speedL = 0xFFFF;
  } else if (speedL<0) {
    speedL = 0;
  }
  if (speedR>0xFFFF) {
    speedR = 0xFFFF;
  } else if (speedR<0) {
    speedR = 0;
  }
  /* send new speed values to motor */
  Motor_SetDirection(Motor_GetMotorHandle(Motor_MOTOR_LEFT), directionL);
  Motor_SetDirection(Motor_GetMotorHandle(Motor_MOTOR_RIGHT), directionR);
  Motor_SetVal(Motor_GetMotorHandle(Motor_MOTOR_LEFT), 0xFFFF-speedL); /* PWM is low active */
  Motor_SetVal(Motor_GetMotorHandle(Motor_MOTOR_RIGHT), 0xFFFF-speedR); /* PWM is low active */
}
#endif

#if PL_CONFIG_LINE_FOLLOWING || PL_CONFIG_MAZE_SOLVING
void PID_Line(uint16_t currLinePos, uint16_t setLinePos, uint16_t currLineWidth, bool forward) {
#if PL_CONFIG_USE_LINE_PID
#if PL_GO_DEADEND_BW
  if (forward) {
    PID_LineCfg(currLinePos, setLinePos, forward, &lineFwConfig);
  } else {
    PID_LineCfg(currLinePos, setLinePos, forward, &lineBwConfig);
  }
#else
  (void)forward; /* not used */
  PID_LineCfg(currLinePos, setLinePos, currLineWidth, forward, &lineFwConfig);
#endif
#endif
}
#endif

#if PL_CONFIG_USE_POS_PID
static void PID_PosCfg(int32_t currPos, int32_t setPos, bool isLeft, PID_Config *config) {
  int32_t speed;
  Motor_Direction direction=MOTOR_DIR_FORWARD;
  Motor_MotorDevice *motHandle;

#if PL_HAS_HIGH_RES_ENCODER
  int error;

  error = setPos-currPos;
  if (error>-config->deadBand && error<config->deadBand) { /* avoid jitter around zero */
    setPos = currPos;
  }
#endif
  speed = PID(currPos, setPos, config);
  /* transform into motor speed */
  speed *= 1000; /* scale PID, otherwise we need high PID constants */
  if (speed>=0) {
    direction = MOTOR_DIR_FORWARD;
  } else { /* negative, make it positive */
    speed = -speed; /* make positive */
    direction = MOTOR_DIR_BACKWARD;
  }
  /* speed is now always positive, make sure it is within 16bit PWM boundary */
  if (speed>0xFFFF) {
    speed = 0xFFFF;
  }
  /* limit speed to maximum value */
  speed = (speed*config->maxSpeedPercent)/100;
  /* send new speed values to motor */
  if (isLeft) {
    motHandle = Motor_GetMotorHandle(MOTOR_SIDE_LEFT);
  } else {
    motHandle = Motor_GetMotorHandle(MOTOR_SIDE_RIGHT);
  }
  Motor_SetVal(motHandle, 0xFFFF-speed); /* PWM is low active */
  Motor_SetDirection(motHandle, direction);
  Motor_UpdatePercent(motHandle, direction);
}

void PID_Pos(int32_t currPos, int32_t setPos, bool isLeft) {
  if (isLeft) {
    PID_PosCfg(currPos, setPos, isLeft, &posLeftConfig);
  } else {
    PID_PosCfg(currPos, setPos, isLeft, &posRightConfig);
  }
}
#endif /* PL_CONFIG_USE_POS_PID */

#if PL_CONFIG_USE_SPEED_PID
static void PID_SpeedCfg(int32_t currSpeed, int32_t setSpeed, bool isLeft, PID_Config *config) {
  int32_t speed;
  Motor_Direction direction=MOTOR_DIR_FORWARD;
  Motor_MotorDevice *motHandle;
  
  if (setSpeed==0) { /* Actually this test is more of a hack, should not be needed! */
    speed = 0;
    /* reset PID I and D values */
    config->integral = 0;
    config->lastError = 0;
  } else {
    speed = PID(currSpeed, setSpeed, config);
  }
  if (speed>=0) {
    direction = MOTOR_DIR_FORWARD;
  } else { /* negative, make it positive */
    speed = -speed; /* make positive */
    direction = MOTOR_DIR_BACKWARD;
  }
  /* speed shall be positive here, make sure it is within 16bit PWM boundary */
  if (speed>0xFFFF) {
    speed = 0xFFFF;
  }
  /* send new speed values to motor */
  if (isLeft) {
    motHandle = Motor_GetMotorHandle(MOTOR_SIDE_LEFT);
  } else {
    motHandle = Motor_GetMotorHandle(MOTOR_SIDE_RIGHT);
  }
  Motor_SetVal(motHandle, 0xFFFF-speed); /* PWM is low active */
  Motor_SetDirection(motHandle, direction);
  Motor_UpdatePercent(motHandle, direction);
}

void PID_Speed(int32_t currSpeed, int32_t setSpeed, bool isLeft) {
  if (isLeft) {
    PID_SpeedCfg(currSpeed, setSpeed, isLeft, &speedLeftConfig);
  } else {
    PID_SpeedCfg(currSpeed, setSpeed, isLeft, &speedRightConfig);
  }
}
#endif /* PL_CONFIG_USE_SPEED_PID */

#if PL_CONFIG_USE_SHELL
static void PID_PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"pid", (unsigned char*)"Group of PID commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows PID help or status\r\n", io->stdOut);
#if PL_CONFIG_USE_POS_PID
  McuShell_SendHelpStr((unsigned char*)"  pos (p|i|d|w|b) <v>", (unsigned char*)"Sets position P, I, D, anti-windup, dead-band value\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  pos speed <v>", (unsigned char*)"Maximum speed % value for position PID\r\n", io->stdOut);
#endif
#if PL_CONFIG_USE_SPEED_PID
  McuShell_SendHelpStr((unsigned char*)"  speed (L|R) (p|i|d|w|b) <v>", (unsigned char*)"Sets speed P, I, D, anti-Windup, dead-band value\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  speed (L|R) speed <v>", (unsigned char*)"Maximum speed % value for speed PID\r\n", io->stdOut);
#endif
#if PL_CONFIG_USE_LINE_PID
  McuShell_SendHelpStr((unsigned char*)"  fw (p|i|d|w|b) <v>", (unsigned char*)"Sets line following P, I, D, anti-Windup, dead-band value\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  fw speed <value>", (unsigned char*)"Maximum speed % value for line following\r\n", io->stdOut);
#endif
#if PL_GO_DEADEND_BW
  McuShell_SendHelpStr((unsigned char*)"  bw (p|i|d|w|b) <v>", (unsigned char*)"Sets P, I, D, anti-windup, dead-band backward line following value\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  bw speed <v>", (unsigned char*)"Maximum backward speed % value\r\n", io->stdOut);
#endif
#if 0
  McuShell_SendHelpStr((unsigned char*)"  wall (p|i|d|w|b) <v>", (unsigned char*)"Sets line following P, I, D, anti-Windup, dead-band value\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  wall speed <value>", (unsigned char*)"Maximum speed % value for wall following\r\n", io->stdOut);
#endif
}

#if PL_CONFIG_USE_SPEED_PID || PL_CONFIG_USE_POS_PID || PL_CONFIG_USE_LINE_PID
static void PrintPIDstatus(PID_Config *config, const unsigned char *kindStr, const McuShell_StdIOType *io) {
  unsigned char buf[48];
  unsigned char kindBuf[16];

  McuUtility_strcpy(kindBuf, sizeof(buf), (unsigned char*)"  ");
  McuUtility_strcat(kindBuf, sizeof(buf), kindStr);
  McuUtility_strcat(kindBuf, sizeof(buf), (unsigned char*)" PID");
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"p: ");
  McuUtility_strcatNum32s(buf, sizeof(buf), config->pFactor100);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" i: ");
  McuUtility_strcatNum32s(buf, sizeof(buf), config->iFactor100);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" d: ");
  McuUtility_strcatNum32s(buf, sizeof(buf), config->dFactor100);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr(kindBuf, buf, io->stdOut);

  McuUtility_strcpy(kindBuf, sizeof(buf), (unsigned char*)"  ");
  McuUtility_strcat(kindBuf, sizeof(buf), kindStr);
  McuUtility_strcat(kindBuf, sizeof(buf), (unsigned char*)" windup");
  McuUtility_Num32sToStr(buf, sizeof(buf), config->iAntiWindup);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr(kindBuf, buf, io->stdOut);

  McuUtility_strcpy(kindBuf, sizeof(buf), (unsigned char*)"  ");
  McuUtility_strcat(kindBuf, sizeof(buf), kindStr);
  McuUtility_strcat(kindBuf, sizeof(buf), (unsigned char*)" error");
  McuUtility_Num32sToStr(buf, sizeof(buf), config->lastError);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr(kindBuf, buf, io->stdOut);

  McuUtility_strcpy(kindBuf, sizeof(buf), (unsigned char*)"  ");
  McuUtility_strcat(kindBuf, sizeof(buf), kindStr);
  McuUtility_strcat(kindBuf, sizeof(buf), (unsigned char*)" integral");
  McuUtility_Num32sToStr(buf, sizeof(buf), config->integral);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr(kindBuf, buf, io->stdOut);

  McuUtility_strcpy(kindBuf, sizeof(buf), (unsigned char*)"  ");
  McuUtility_strcat(kindBuf, sizeof(buf), kindStr);
  McuUtility_strcat(kindBuf, sizeof(buf), (unsigned char*)" speed");
  McuUtility_Num8uToStr(buf, sizeof(buf), config->maxSpeedPercent);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"%\r\n");
  McuShell_SendStatusStr(kindBuf, buf, io->stdOut);

  McuUtility_strcpy(kindBuf, sizeof(buf), (unsigned char*)"  ");
  McuUtility_strcat(kindBuf, sizeof(buf), kindStr);
  McuUtility_strcat(kindBuf, sizeof(buf), (unsigned char*)" band");
  McuUtility_Num8uToStr(buf, sizeof(buf), config->deadBand);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr(kindBuf, buf, io->stdOut);
}
#endif

static void PID_PrintStatus(const McuShell_StdIOType *io) {
#if PL_CONFIG_USE_LINE_PID || PL_GO_DEADEND_BW || PL_CONFIG_USE_POS_PID || PL_CONFIG_USE_SPEED_PID
  McuShell_SendStatusStr((unsigned char*)"pid", (unsigned char*)"PID control loop status\r\n", io->stdOut);
#endif
#if PL_CONFIG_USE_LINE_PID
  PrintPIDstatus(&lineFwConfig, (unsigned char*)"fw", io);
#endif
#if PL_GO_DEADEND_BW
  PrintPIDstatus(&lineBwConfig, (unsigned char*)"bw", io);
#endif
#if PL_CONFIG_USE_POS_PID
  PrintPIDstatus(&posLeftConfig, (unsigned char*)"pos L", io);
  PrintPIDstatus(&posRightConfig, (unsigned char*)"pos R", io);
#endif  
#if PL_CONFIG_USE_SPEED_PID
  PrintPIDstatus(&speedLeftConfig, (unsigned char*)"speed L", io);
  PrintPIDstatus(&speedRightConfig, (unsigned char*)"speed R", io);
#endif
}

#if PL_CONFIG_USE_SPEED_PID || PL_CONFIG_USE_POS_PID || PL_CONFIG_USE_LINE_PID
static uint8_t ParsePidParameter(PID_Config *config, const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  const unsigned char *p;
  uint32_t val32u;
  uint8_t val8u;
  uint8_t res = ERR_OK;

  if (McuUtility_strncmp((char*)cmd, (char*)"p ", sizeof("p ")-1)==0) {
    p = cmd+sizeof("p");
    if (McuUtility_ScanDecimal32uNumber(&p, &val32u)==ERR_OK) {
      config->pFactor100 = val32u;
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument\r\n", io->stdErr);
      res = ERR_FAILED;
    }
  } else if (McuUtility_strncmp((char*)cmd, (char*)"i ", sizeof("i ")-1)==0) {
    p = cmd+sizeof("i");
    if (McuUtility_ScanDecimal32uNumber(&p, &val32u)==ERR_OK) {
      config->iFactor100 = val32u;
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument\r\n", io->stdErr);
      res = ERR_FAILED;
    }
  } else if (McuUtility_strncmp((char*)cmd, (char*)"d ", sizeof("d ")-1)==0) {
    p = cmd+sizeof("d");
    if (McuUtility_ScanDecimal32uNumber(&p, &val32u)==ERR_OK) {
      config->dFactor100 = val32u;
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument\r\n", io->stdErr);
      res = ERR_FAILED;
    }
  } else if (McuUtility_strncmp((char*)cmd, (char*)"w ", sizeof("w ")-1)==0) {
    p = cmd+sizeof("w");
    if (McuUtility_ScanDecimal32uNumber(&p, &val32u)==ERR_OK) {
      config->iAntiWindup = val32u;
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument\r\n", io->stdErr);
      res = ERR_FAILED;
    }
  } else if (McuUtility_strncmp((char*)cmd, (char*)"b ", sizeof("b ")-1)==0) {
    p = cmd+sizeof("b");
    if (McuUtility_ScanDecimal32uNumber(&p, &val32u)==ERR_OK) {
      config->deadBand = val32u;
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument\r\n", io->stdErr);
      res = ERR_FAILED;
    }
  } else if (McuUtility_strncmp((char*)cmd, (char*)"speed ", sizeof("speed ")-1)==0) {
    p = cmd+sizeof("speed");
    if (McuUtility_ScanDecimal8uNumber(&p, &val8u)==ERR_OK && val8u<=100) {
      config->maxSpeedPercent = val8u;
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument\r\n", io->stdErr);
      res = ERR_FAILED;
    }
  }
  return res;
}
#endif

uint8_t PID_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  uint8_t res = ERR_OK;

  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"pid help")==0) {
    PID_PrintHelp(io);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"pid status")==0) {
    PID_PrintStatus(io);
    *handled = TRUE;
#if PL_CONFIG_USE_LINE_PID
  } else if (McuUtility_strncmp((char*)cmd, (char*)"pid fw ", sizeof("pid fw ")-1)==0) {
    res = ParsePidParameter(&lineFwConfig, cmd+sizeof("pid fw ")-1, handled, io);
#endif
#if PL_GO_DEADEND_BW
  } else if (McuUtility_strncmp((char*)cmd, (char*)"pid bw ", sizeof("pid bw ")-1)==0) {
    res = ParsePidParameter(&lineBwConfig, cmd+sizeof("pid bw ")-1, handled, io);
#endif
#if PL_CONFIG_USE_POS_PID
  } else if (McuUtility_strncmp((char*)cmd, (char*)"pid pos ", sizeof("pid pos ")-1)==0) {
    res = ParsePidParameter(&posLeftConfig, cmd+sizeof("pid pos ")-1, handled, io);
    if (res==ERR_OK) {
      res = ParsePidParameter(&posRightConfig, cmd+sizeof("pid pos ")-1, handled, io);
    }
#endif
#if PL_CONFIG_USE_SPEED_PID
  } else if (McuUtility_strncmp((char*)cmd, (char*)"pid speed L ", sizeof("pid speed L ")-1)==0) {
    res = ParsePidParameter(&speedLeftConfig, cmd+sizeof("pid speed L ")-1, handled, io);
  } else if (McuUtility_strncmp((char*)cmd, (char*)"pid speed R ", sizeof("pid speed R ")-1)==0) {
    res = ParsePidParameter(&speedRightConfig, cmd+sizeof("pid speed R ")-1, handled, io);
#endif
  }
  return res;
}
#endif /* PL_CONFIG_USE_SHELL */

void PID_Start(void) {
#if PL_CONFIG_USE_LINE_PID
  lineFwConfig.lastError = 0;
  lineFwConfig.integral = 0;
#endif
#if PL_GO_DEADEND_BW
  lineBwConfig.lastError = 0;
  lineBwConfig.integral = 0;
#endif
#if PL_CONFIG_USE_POS_PID
  posLeftConfig.lastError = 0;
  posLeftConfig.integral = 0;
  posRightConfig.lastError = 0;
  posRightConfig.integral = 0;
#endif
#if PL_CONFIG_USE_SPEED_PID
  speedLeftConfig.lastError = 0;
  speedLeftConfig.integral = 0;
  speedRightConfig.lastError = 0;
  speedRightConfig.integral = 0;
#endif
}

void PID_Deinit(void) {
}

void PID_Init(void) {
#if PL_CONFIG_USE_LINE_PID
  lineFwConfig.pFactor100 = PID_CONFIG_LINE_P;
  lineFwConfig.iFactor100 = PID_CONFIG_LINE_I;
  lineFwConfig.dFactor100 = PID_CONFIG_LINE_D;
  lineFwConfig.iAntiWindup = PID_CONFIG_LINE_W;
  lineFwConfig.maxSpeedPercent = PID_CONFIG_LINE_V;
  lineFwConfig.deadBand = PID_CONFIG_LINE_B;
  lineFwConfig.lastError = 0;
  lineFwConfig.integral = 0;
#if PL_GO_DEADEND_BW
  lineBwConfig.pFactor100 = PID_CONFIG_LINE_BACK_P;
  lineBwConfig.iFactor100 = PID_CONFIG_LINE_BACK_I;
  lineBwConfig.dFactor100 = PID_CONFIG_LINE_BACK_D;
  lineBwConfig.iAntiWindup = PID_CONFIG_LINE_BACK_W;
  lineBwConfig.maxSpeedPercent = PID_CONFIG_LINE_BACK_V;
  lineFwConfig.deadBand = PID_CONFIG_LINE_BACK_B;
  lineBwConfig.lastError = 0;
  lineBwConfig.integral = 0;
#endif
#endif /* PL_CONFIG_USE_LINE_PID */
  
#if PL_CONFIG_USE_POS_PID
  posLeftConfig.pFactor100 = PID_CONFIG_POS_P;
  posLeftConfig.iFactor100 = PID_CONFIG_POS_I;
  posLeftConfig.dFactor100 = PID_CONFIG_POS_D;
  posLeftConfig.iAntiWindup = PID_CONFIG_POS_W;
  posLeftConfig.maxSpeedPercent = PID_CONFIG_POS_V;
  posLeftConfig.deadBand = PID_CONFIG_POS_B;
  posLeftConfig.lastError = 0;
  posLeftConfig.integral = 0;
  posRightConfig = posLeftConfig; /* struct copy */
#endif

#if PL_CONFIG_USE_SPEED_PID
  speedLeftConfig.pFactor100 = PID_CONFIG_SPEED_P;
  speedLeftConfig.iFactor100 = PID_CONFIG_SPEED_I;
  speedLeftConfig.dFactor100 = PID_CONFIG_SPEED_D;
  speedLeftConfig.iAntiWindup = PID_CONFIG_SPEED_W;
  speedLeftConfig.maxSpeedPercent = PID_CONFIG_SPEED_V;
  speedLeftConfig.lastError = 0;
  speedLeftConfig.integral = 0;
  speedLeftConfig.deadBand = 0;
  speedRightConfig = speedLeftConfig; /* struct copy */
#endif
}
#endif /* PL_CONFIG_USE_PID */
