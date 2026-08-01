/*
 * Copyright (c) 2019, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_ADOPT_HW
#include "identify.h"
#include "motor.h"
#include "quadCounter.h"
#include "McuLog.h"
#include <stdbool.h>

typedef enum {
  ROBOT_HW_V1,
  ROBOT_HW_V2
} RobotHW_Version;

typedef struct {
  ID_Robot_e id;
  RobotHW_Version version;
  bool invertDirLeft, invertDirRight;
  bool swapQuadLeft, swapQuadRight;
} RobotHW_Config;

#if PL_CONFIG_USE_MOTORS
static const RobotHW_Config RobotHWConfigTable[] =
{ /*! \todo The following table needs to be completed and verified for all robots */
  {.id=ID_ROBOT_E0,  .version=ROBOT_HW_V2, .invertDirLeft=TRUE,   .invertDirRight=TRUE,  .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_E1,  .version=ROBOT_HW_V2, .invertDirLeft=FALSE,  .invertDirRight=FALSE, .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_E2,  .version=ROBOT_HW_V2, .invertDirLeft=FALSE,  .invertDirRight=FALSE, .swapQuadLeft=FALSE, .swapQuadRight=FALSE },
  {.id=ID_ROBOT_E3,  .version=ROBOT_HW_V2, .invertDirLeft=FALSE,  .invertDirRight=FALSE, .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified*/
  {.id=ID_ROBOT_E4,  .version=ROBOT_HW_V2, .invertDirLeft=FALSE,  .invertDirRight=FALSE, .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified*/
  {.id=ID_ROBOT_E5,  .version=ROBOT_HW_V2, .invertDirLeft=FALSE,  .invertDirRight=FALSE, .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, 
  {.id=ID_ROBOT_E6,  .version=ROBOT_HW_V1, .invertDirLeft=FALSE,  .invertDirRight=FALSE, .swapQuadLeft=TRUE,  .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_E7,  .version=ROBOT_HW_V2, .invertDirLeft=FALSE,  .invertDirRight=TRUE,  .swapQuadLeft=FALSE, .swapQuadRight=FALSE },
  {.id=ID_ROBOT_E8,  .version=ROBOT_HW_V2, .invertDirLeft=FALSE,  .invertDirRight=FALSE, .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_E9,  .version=ROBOT_HW_V1, .invertDirLeft=TRUE,   .invertDirRight=FALSE, .swapQuadLeft=TRUE,  .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_E10, .version=ROBOT_HW_V1, .invertDirLeft=TRUE,   .invertDirRight=FALSE, .swapQuadLeft=TRUE,  .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_E11, .version=ROBOT_HW_V2, .invertDirLeft=TRUE,   .invertDirRight=TRUE,  .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_E12, .version=ROBOT_HW_V1, .invertDirLeft=TRUE,   .invertDirRight=FALSE, .swapQuadLeft=TRUE,  .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_E13, .version=ROBOT_HW_V2, .invertDirLeft=FALSE,  .invertDirRight=TRUE,  .swapQuadLeft=FALSE, .swapQuadRight=FALSE },
  {.id=ID_ROBOT_E14, .version=ROBOT_HW_V2, .invertDirLeft=true,   .invertDirRight=false, .swapQuadLeft=FALSE, .swapQuadRight=FALSE },
  {.id=ID_ROBOT_E17, .version=ROBOT_HW_V2, .invertDirLeft=TRUE,   .invertDirRight=FALSE, .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_E18, .version=ROBOT_HW_V1, .invertDirLeft=TRUE,   .invertDirRight=FALSE, .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_E27, .version=ROBOT_HW_V2, .invertDirLeft=TRUE,   .invertDirRight=TRUE,  .swapQuadLeft=FALSE, .swapQuadRight=FALSE },

  {.id=ID_ROBOT_E34, .version=ROBOT_HW_V2, .invertDirLeft=FALSE,  .invertDirRight=TRUE,  .swapQuadLeft=FALSE, .swapQuadRight=FALSE },

  {.id=ID_ROBOT_L0,  .version=ROBOT_HW_V2, .invertDirLeft=FALSE,  .invertDirRight=FALSE, .swapQuadLeft=FALSE, .swapQuadRight=FALSE },
  {.id=ID_ROBOT_L1,  .version=ROBOT_HW_V1, .invertDirLeft=TRUE,   .invertDirRight=FALSE, .swapQuadLeft=TRUE,  .swapQuadRight=FALSE }, /*verified*/
  {.id=ID_ROBOT_L3,  .version=ROBOT_HW_V1, .invertDirLeft=FALSE,  .invertDirRight=TRUE,  .swapQuadLeft=FALSE, .swapQuadRight=FALSE },
  {.id=ID_ROBOT_L17, .version=ROBOT_HW_V2, .invertDirLeft=TRUE,   .invertDirRight=TRUE,  .swapQuadLeft=FALSE, .swapQuadRight=FALSE },	/* verified */
  {.id=ID_ROBOT_L20, .version=ROBOT_HW_V2, .invertDirLeft=TRUE,   .invertDirRight=TRUE,  .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified */

  {.id=ID_ROBOT_R0,  .version=ROBOT_HW_V2, .invertDirLeft=TRUE,   .invertDirRight=TRUE,  .swapQuadLeft=FALSE, .swapQuadRight=FALSE },
  {.id=ID_ROBOT_R8,  .version=ROBOT_HW_V1, .invertDirLeft=FALSE,  .invertDirRight=FALSE, .swapQuadLeft=TRUE,  .swapQuadRight=FALSE },
  {.id=ID_ROBOT_R9,  .version=ROBOT_HW_V1, .invertDirLeft=FALSE,  .invertDirRight=FALSE, .swapQuadLeft=TRUE,  .swapQuadRight=FALSE },	/* verified */
  {.id=ID_ROBOT_R23, .version=ROBOT_HW_V2, .invertDirLeft=TRUE,   .invertDirRight=TRUE,  .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_R27, .version=ROBOT_HW_V2, .invertDirLeft=FALSE,  .invertDirRight=TRUE,  .swapQuadLeft=FALSE, .swapQuadRight=FALSE },
  {.id=ID_ROBOT_R28, .version=ROBOT_HW_V2, .invertDirLeft=FALSE,  .invertDirRight=TRUE,  .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_R29, .version=ROBOT_HW_V2, .invertDirLeft=FALSE,  .invertDirRight=FALSE, .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_R32, .version=ROBOT_HW_V2, .invertDirLeft=TRUE,   .invertDirRight=TRUE,  .swapQuadLeft=FALSE, .swapQuadRight=FALSE },
  {.id=ID_ROBOT_R33, .version=ROBOT_HW_V2, .invertDirLeft=FALSE,  .invertDirRight=TRUE,  .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_R34, .version=ROBOT_HW_V2, .invertDirLeft=FALSE,  .invertDirRight=FALSE, .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_R36, .version=ROBOT_HW_V2, .invertDirLeft=TRUE,   .invertDirRight=TRUE,  .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_R37, .version=ROBOT_HW_V2, .invertDirLeft=TRUE,   .invertDirRight=FALSE, .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_R44, .version=ROBOT_HW_V2, .invertDirLeft=TRUE,   .invertDirRight=TRUE,  .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified */
  {.id=ID_ROBOT_R45, .version=ROBOT_HW_V2, .invertDirLeft=TRUE,   .invertDirRight=FALSE, .swapQuadLeft=FALSE, .swapQuadRight=FALSE }, /* verified */
};
#endif

#if PL_CONFIG_USE_MOTORS
static const RobotHW_Config *GetRobotHWDesc(ID_Robot_e id) {
  unsigned int i;

  for(i=0; i<sizeof(RobotHWConfigTable)/sizeof(RobotHWConfigTable[0]); i++) {
    if (RobotHWConfigTable[i].id == id) {
      return &RobotHWConfigTable[i]; /* found it */
    }
  }
  return NULL;
}
#endif

void ADAPT_AdaptToHardware(void) {
#if PL_CONFIG_USE_IDENTIFY
#if PL_CONFIG_USE_MOTORS
  ID_Robot_e robot;
  const RobotHW_Config *config;

  #if !PL_CONFIG_USE_QUADRATURE || !PL_CONFIG_USE_MOTORS
    #error "both quadrature counter pins and motor driver pins must be enabled and initialized"
  #endif

  robot = ID_WhichDevice();
  if (robot!=ID_ROBOT_NONE) {
    config = GetRobotHWDesc(robot);
    if (config==NULL) {
      McuLog_error("robot with ID %d not found", robot);
      return;
    }
    if (config->invertDirLeft) {
      Motor_Invert(Motor_GetMotorHandle(MOTOR_SIDE_LEFT), TRUE); /* invert motor */
    }
    if (config->invertDirRight) {
      Motor_Invert(Motor_GetMotorHandle(MOTOR_SIDE_RIGHT), TRUE); /* invert motor */
    }
    if (config->swapQuadLeft) {
      QuadCounter_SwapPinsLeft(true);
    }
    if (config->swapQuadRight) {
      QuadCounter_SwapPinsRight(true);
    }
  }
  if (config->version==ROBOT_HW_V2) {
    QuadCounter_EnablePullups(); /* pull-ups for Quadrature Encoder Pins */
  }
#endif
#endif
}

#endif /* #if PL_CONFIG_USE_ADOPT_HW */
