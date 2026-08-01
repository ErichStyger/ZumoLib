/**
 * \file
 * \brief Motor driver implementation.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * This module a driver for up to two small DC motors.
 */

#include "platform.h"
#if PL_CONFIG_USE_MOTORS
#include "motor.h"
#include "McuGPIO.h"
#include "McuUtility.h"
#include "fsl_ftm.h"
#include "fsl_port.h"

static Motor_MotorDevice motorL, motorR;
static bool isMotorOn = true;
static McuGPIO_Handle_t Motor_DirL, Motor_DirR;
#if MOTOR_CONFIG_HAS_POWER_ON
  static McuGPIO_Handle_t Motor_PowerOn;
#endif
#if MOTOR_CONFIG_HAS_MODE
  static McuGPIO_Handle_t Motor_Mode;
#endif

void TMR_SetLeftMotorPWMDutyPercent(uint8_t percent) {
  FTM_UpdatePwmDutycycle(MOTOR_PWM_FTM_BASEADDR, (ftm_chnl_t)MOTOR_PWM_LEFT_FTM_CHANNEL, kFTM_EdgeAlignedPwm, 100-percent);
  /* Software trigger to update registers */
  FTM_SetSoftwareTrigger(MOTOR_PWM_FTM_BASEADDR, true);
}

void TMR_SetRightMotorPWMDutyPercent(uint8_t percent) {
  FTM_UpdatePwmDutycycle(MOTOR_PWM_FTM_BASEADDR, (ftm_chnl_t)MOTOR_PWM_RIGHT_FTM_CHANNEL, kFTM_EdgeAlignedPwm, 100-percent);
  /* Software trigger to update registers */
  FTM_SetSoftwareTrigger(MOTOR_PWM_FTM_BASEADDR, true);
}

void TMR_SetLeftMotorPWMRatio(uint16_t val) {
  TMR_SetLeftMotorPWMDutyPercent(val>>8);
}

void TMR_SetRightMotorPWMRatio(uint16_t val) {
  TMR_SetRightMotorPWMDutyPercent(val>>8);
}

void TMR_MotorPWMStart(void) {
  FTM_StartTimer(MOTOR_PWM_FTM_BASEADDR, kFTM_SystemClock);
}

void TMR_MotorPWMStop(void) {
  FTM_StopTimer(MOTOR_PWM_FTM_BASEADDR);
}

static void InitMotorPWMTimer(void) {
  ftm_chnl_pwm_signal_param_t ftmParam[2];
  ftm_config_t ftmInfo;

  /* Configure ftm params with frequency 24kHZ */
  ftmParam[0].chnlNumber = (ftm_chnl_t)MOTOR_PWM_LEFT_FTM_CHANNEL;
  ftmParam[0].level = kFTM_LowTrue;
  ftmParam[0].dutyCyclePercent = 0U;
  ftmParam[0].firstEdgeDelayPercent = 0U;

  ftmParam[1].chnlNumber = (ftm_chnl_t)MOTOR_PWM_RIGHT_FTM_CHANNEL;
  ftmParam[1].level = kFTM_LowTrue;
  ftmParam[1].dutyCyclePercent = 0U;
  ftmParam[1].firstEdgeDelayPercent = 0U;

  /*
    * ftmInfo.prescale = kFTM_Prescale_Divide_1;
    * ftmInfo.bdmMode = kFTM_BdmMode_0;
    * ftmInfo.pwmSyncMode = kFTM_SoftwareTrigger;
    * ftmInfo.reloadPoints = 0;
    * ftmInfo.faultMode = kFTM_Fault_Disable;
    * ftmInfo.faultFilterValue = 0;
    * ftmInfo.deadTimePrescale = kFTM_Deadtime_Prescale_1;
    * ftmInfo.deadTimeValue = 0;
    * ftmInfo.extTriggers = 0;
    * ftmInfo.chnlInitState = 0;
    * ftmInfo.chnlPolarity = 0;
    * ftmInfo.useGlobalTimeBase = false;
    */
   FTM_GetDefaultConfig(&ftmInfo);
   /* Initialize FTM module */
   FTM_Init(MOTOR_PWM_FTM_BASEADDR, &ftmInfo);

   FTM_SetupPwm(MOTOR_PWM_FTM_BASEADDR, ftmParam, 2U, kFTM_EdgeAlignedPwm, 24000U, MOTOR_FTM_SOURCE_CLOCK);
   TMR_SetLeftMotorPWMDutyPercent(0);
   TMR_SetRightMotorPWMDutyPercent(0);
   //FTM_StartTimer(MOTOR_PWM_FTM_BASEADDR, kFTM_SystemClock);
}

Motor_MotorDevice *Motor_GetMotorHandle(Motor_MotorSide side) {
  if (side==MOTOR_SIDE_LEFT) {
    return &motorL;
  } else {
    return &motorR;
  }
}

static void PWMLSetRatio16(uint16_t ratio) {
  /* lower ratio is higher speed */
  ratio = 0xffff-ratio; /* because already inverted */
  ratio = ratio/(0xffff/100); /* scale to 0...100 */
  TMR_SetLeftMotorPWMDutyPercent(ratio);
}

static void PWMRSetRatio16(uint16_t ratio) {
  ratio = 0xffff-ratio; /* because already inverted */
  ratio = ratio/(0xffff/100); /* scale to 0...100 */
  TMR_SetRightMotorPWMDutyPercent(ratio);
}

static void DirLPutVal(bool val) {
  McuGPIO_SetValue(Motor_DirL, val);
}

static void DirRPutVal(bool val) {
  McuGPIO_SetValue(Motor_DirR, val);
}

void Motor_SetVal(Motor_MotorDevice *motor, uint16_t val) {
  if (isMotorOn) {
    motor->currPWMvalue = val;
    (void)motor->SetRatio16(val);
  } else { /* have motor stopped */
    motor->currPWMvalue = 0xFFFF;
    (void)motor->SetRatio16(0xFFFF);
  }
}

void Motor_OnOff(bool on) {
  isMotorOn = on;
  if (!on) {
    Motor_SetSpeedPercent(Motor_GetMotorHandle(MOTOR_SIDE_LEFT), 0);
    Motor_SetSpeedPercent(Motor_GetMotorHandle(MOTOR_SIDE_RIGHT), 0);
  }
}

uint16_t Motor_GetVal(Motor_MotorDevice *motor) {
  return motor->currPWMvalue;
}

#if MOTOR_HAS_INVERT
void Motor_Invert(Motor_MotorDevice *motor, bool inverted) {
  motor->inverted = inverted;
}
#endif


void Motor_SetSpeedPercent(Motor_MotorDevice *motor, Motor_SpeedPercent percent) {
  uint32_t val;

  if (percent>100) { /* make sure we are within 0..100 */
    percent = 100;
  } else if (percent<-100) {
    percent = -100;
  }
  motor->currSpeedPercent = percent; /* store value */
  if (percent<0) {
    Motor_SetDirection(motor, MOTOR_DIR_BACKWARD);
    percent = -percent; /* make it positive */
  } else {
    Motor_SetDirection(motor, MOTOR_DIR_FORWARD);
  }
  val = ((100-percent)*0xffff)/100; /* H-Bridge is low active! */
  Motor_SetVal(motor, (uint16_t)val);
}

void Motor_UpdatePercent(Motor_MotorDevice *motor, Motor_Direction dir) {
  motor->currSpeedPercent = ((0xffff-motor->currPWMvalue)*100)/0xffff;
  if (dir==MOTOR_DIR_BACKWARD) {
    motor->currSpeedPercent = -motor->currSpeedPercent;
  }
}

Motor_SpeedPercent Motor_GetSpeedPercent(Motor_MotorDevice *motor) {
  return motor->currSpeedPercent;
}

void Motor_ChangeSpeedPercent(Motor_MotorDevice *motor, Motor_SpeedPercent relPercent) {
  relPercent += motor->currSpeedPercent; /* make absolute number */
  if (relPercent>100) { /* check for overflow */
    relPercent = 100;
  } else if (relPercent<-100) { /* and underflow */
    relPercent = -100;
  }
  Motor_SetSpeedPercent(motor, relPercent);  /* set speed. This will care about the direction too */
}

void Motor_SetDirection(Motor_MotorDevice *motor, Motor_Direction dir) {
  if (dir==MOTOR_DIR_FORWARD ) {
#if MOTOR_HAS_INVERT
    motor->DirPutVal(motor->inverted?0:1);
#else
    motor->DirPutVal(1);
#endif
    if (motor->currSpeedPercent<0) {
      motor->currSpeedPercent = -motor->currSpeedPercent;
    }
  } else if (dir==MOTOR_DIR_BACKWARD) {
#if MOTOR_HAS_INVERT
    motor->DirPutVal(motor->inverted?1:0);
#else
    motor->DirPutVal(0);
#endif
    if (motor->currSpeedPercent>0) {
      motor->currSpeedPercent = -motor->currSpeedPercent;
    }
  }
}

Motor_Direction Motor_GetDirection(Motor_MotorDevice *motor) {
  if (motor->currSpeedPercent<0) {
    return MOTOR_DIR_BACKWARD;
  } else {
    return MOTOR_DIR_FORWARD;
  }
}

#if PL_CONFIG_USE_SHELL
static void Motor_PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"motor", (unsigned char*)"Group of motor commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows motor help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  on|off", (unsigned char*)"Enables or disables motor\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  (L|R) forward|backward", (unsigned char*)"Change motor direction\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  (L|R) duty <number>", (unsigned char*)"Change motor PWM (-100..+100)%\r\n", io->stdOut);
}

static void Motor_PrintStatus(const McuShell_StdIOType *io) {
  unsigned char buf[32];

  McuShell_SendStatusStr((unsigned char*)"Motor", (unsigned char*)"Motor status\r\n", io->stdOut);
#ifdef MOTOR_HAS_INVERT
  McuShell_SendStatusStr((unsigned char*)"  inverted L", Motor_GetMotorHandle(MOTOR_SIDE_LEFT)->inverted?(unsigned char*)"yes\r\n":(unsigned char*)"no\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  inverted R", Motor_GetMotorHandle(MOTOR_SIDE_RIGHT)->inverted?(unsigned char*)"yes\r\n":(unsigned char*)"no\r\n", io->stdOut);
#endif
  McuShell_SendStatusStr((unsigned char*)"  on/off", isMotorOn?(unsigned char*)"on\r\n":(unsigned char*)"off\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  motor L", (unsigned char*)"", io->stdOut);
  buf[0] = '\0';
  McuUtility_Num16sToStrFormatted(buf, sizeof(buf), (int16_t)motorL.currSpeedPercent, ' ', 4);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"% 0x");
  McuUtility_strcatNum16Hex(buf, sizeof(buf), Motor_GetVal(&motorL));
  McuUtility_strcat(buf, sizeof(buf),(unsigned char*)(Motor_GetDirection(&motorL)==MOTOR_DIR_FORWARD?", fw":", bw"));
  McuShell_SendStr(buf, io->stdOut);
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);

  McuShell_SendStatusStr((unsigned char*)"  motor R", (unsigned char*)"", io->stdOut);
  buf[0] = '\0';
  McuUtility_Num16sToStrFormatted(buf, sizeof(buf), (int16_t)motorR.currSpeedPercent, ' ', 4);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"% 0x");
  McuUtility_strcatNum16Hex(buf, sizeof(buf), Motor_GetVal(&motorR));
  McuUtility_strcat(buf, sizeof(buf),(unsigned char*)(Motor_GetDirection(&motorR)==MOTOR_DIR_FORWARD?", fw":", bw"));
  McuShell_SendStr(buf, io->stdOut);
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
}

uint8_t Motor_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  uint8_t res = ERR_OK;
  int32_t val;
  const unsigned char *p;

  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"motor help")==0) {
    Motor_PrintHelp(io);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"motor status")==0) {
    Motor_PrintStatus(io);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"motor L forward")==0) {
    Motor_SetDirection(&motorL, MOTOR_DIR_FORWARD);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"motor R forward")==0) {
    Motor_SetDirection(&motorR, MOTOR_DIR_FORWARD);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"motor L backward")==0) {
    Motor_SetDirection(&motorL, MOTOR_DIR_BACKWARD);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"motor R backward")==0) {
    Motor_SetDirection(&motorR, MOTOR_DIR_BACKWARD);
    *handled = TRUE;
  } else if (McuUtility_strncmp((char*)cmd, (char*)"motor L duty ", sizeof("motor L duty ")-1)==0) {
    if (!isMotorOn) {
      McuShell_SendStr((unsigned char*)"Motor is OFF, cannot set duty.\r\n", io->stdErr);
      res = ERR_FAILED;
    } else {
      p = cmd+sizeof("motor L duty");
      if (McuUtility_xatoi(&p, &val)==ERR_OK && val >=-100 && val<=100) {
        Motor_SetSpeedPercent(&motorL, (Motor_SpeedPercent)val);
        *handled = TRUE;
      } else {
        McuShell_SendStr((unsigned char*)"Wrong argument, must be in the range -100..100\r\n", io->stdErr);
        res = ERR_FAILED;
      }
    }
  } else if (McuUtility_strncmp((char*)cmd, (char*)"motor R duty ", sizeof("motor R duty ")-1)==0) {
    if (!isMotorOn) {
      McuShell_SendStr((unsigned char*)"Motor is OFF, cannot set duty.\r\n", io->stdErr);
      res = ERR_FAILED;
    } else {
      p = cmd+sizeof("motor R duty");
      if (McuUtility_xatoi(&p, &val)==ERR_OK && val >=-100 && val<=100) {
        Motor_SetSpeedPercent(&motorR, (Motor_SpeedPercent)val);
        *handled = TRUE;
      } else {
        McuShell_SendStr((unsigned char*)"Wrong argument, must be in the range -100..100\r\n", io->stdErr);
        res = ERR_FAILED;
      }
    }
  } else if (McuUtility_strncmp((char*)cmd, (char*)"motor on", sizeof("motor on")-1)==0) {
    isMotorOn = TRUE;
    *handled = TRUE;
  } else if (McuUtility_strncmp((char*)cmd, (char*)"motor off", sizeof("motor off")-1)==0) {
    Motor_SetSpeedPercent(&motorL, 0);
    Motor_SetSpeedPercent(&motorR, 0);
    isMotorOn = FALSE;
    *handled = TRUE;
  }
  return res;
}
#endif /* PL_CONFIG_USE_SHELL */

void Motor_Deinit(void) {
  Motor_DirL = McuGPIO_DeinitGPIO(Motor_DirL);
  Motor_DirR = McuGPIO_DeinitGPIO(Motor_DirR);
}

static void Motor_DirPinInit(void) {
  McuGPIO_Config_t gpioConfig;

  MOTOR_DIR_ENABLE_CLOCK();
  McuGPIO_GetDefaultConfig(&gpioConfig);
  gpioConfig.isInput = false;
  gpioConfig.isHighOnInit = true;
  gpioConfig.hw.gpio = MOTOR_PINS_DIR_LEFT_GPIO;
  gpioConfig.hw.port = MOTOR_PINS_DIR_LEFT_PORT;
  gpioConfig.hw.pin = MOTOR_PINS_DIR_LEFT_PIN;
  Motor_DirL = McuGPIO_InitGPIO(&gpioConfig);
  if (Motor_DirL==NULL) {
    for(;;) {}
  }

  gpioConfig.hw.gpio = MOTOR_PINS_DIR_RIGHT_GPIO;
  gpioConfig.hw.port = MOTOR_PINS_DIR_RIGHT_PORT;
  gpioConfig.hw.pin = MOTOR_PINS_DIR_RIGHT_PIN;
  Motor_DirR = McuGPIO_InitGPIO(&gpioConfig);
  if (Motor_DirR==NULL) {
    for(;;) {}
  }

#if MOTOR_CONFIG_HAS_POWER_ON
  MOTOR_PINS_POWER_ON_ENABLE_CLOCK();
  gpioConfig.hw.gpio = MOTOR_PINS_POWER_ON_GPIO;
  gpioConfig.hw.port = MOTOR_PINS_POWER_ON_PORT;
  gpioConfig.hw.pin = MOTOR_PINS_POWER_ON_PIN;
  gpioConfig.isHighOnInit = true; /* turned on by default, is high active */
  Motor_PowerOn = McuGPIO_InitGPIO(&gpioConfig);
  if (Motor_PowerOn==NULL) {
    for(;;) {}
  }
#endif

#if MOTOR_CONFIG_HAS_MODE
  MOTOR_PINS_MODE_ENABLE_CLOCK();
  gpioConfig.hw.gpio = MOTOR_PINS_MODE_GPIO;
  gpioConfig.hw.port = MOTOR_PINS_MODE_PORT;
  gpioConfig.hw.pin = MOTOR_PINS_MODE_PIN;
  gpioConfig.isHighOnInit = true; /* set MODE to HIGH, using PWM and DIR signal */
  Motor_Mode = McuGPIO_InitGPIO(&gpioConfig);
  if (Motor_Mode==NULL) {
    for(;;) {}
  }
#endif
}

void Motor_Init(void) {
  /* PWM pins are muxed using the pins tool! */
  Motor_DirPinInit();
  MOTOR_INIT_PWM_PINS();
#if MOTOR_HAS_INVERT
  motorL.inverted = FALSE;
  motorR.inverted = FALSE;
#endif
  motorL.DirPutVal = DirLPutVal;
  motorR.DirPutVal = DirRPutVal;
  InitMotorPWMTimer();
  motorL.SetRatio16 = PWMLSetRatio16;
  motorR.SetRatio16 = PWMRSetRatio16;
  Motor_SetSpeedPercent(&motorL, 0);
  Motor_SetSpeedPercent(&motorR, 0);
  TMR_MotorPWMStart();
}

#endif /* PL_CONFIG_USE_MOTORS */
