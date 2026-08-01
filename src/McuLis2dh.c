/*
 * Copyright David Huwyler, 2019 & Erich Styger, 2026
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file
 * \brief Interface for the ST LIS2DH Accelerometer via I2C.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * \note Initial implementation by David Huwyler for LiDo
 */
#define PL_CONFIG_HAS_ACCEL_INT (0)



#include "McuLis2dh.h"
#include "McuGenericI2C.h"
#include "McuWait.h"
#include "McuUtility.h"
#if PL_CONFIG_HAS_ACCEL_INT
  #include "ACC_INT1.h"
#endif

#if MCU_LIS2DH_CONFIG_IS_ENABLED

#define ACCEL_SENS_I2C_ADDRESS              0x19
#define ACCEL_SENS_I2C_REGISTER_WHOAMI      0x0F
#define ACCEL_SENS_I2C_REGISTER_CTRL_REG1   0x20 // SamplingFrequenz und LowpowerMode (Datasheet p33)
#define ACCEL_SENS_I2C_REGISTER_CTRL_REG2   0x21 // LowPass filter.. (Datasheet p34)
#define ACCEL_SENS_I2C_REGISTER_CTRL_REG3   0x22 // Interrupts (Datasheet p34)
#define ACCEL_SENS_I2C_REGISTER_CTRL_REG4   0x23 // HighResMode, Selftest .. (Datasheet p35)
#define ACCEL_SENS_I2C_REGISTER_CTRL_REG5   0x24
#define ACCEL_SENS_I2C_REGISTER_CTRL_REG6   0x25
#define ACCEL_SENS_I2C_REGISTER_REFERENCE   0x26
#define ACCEL_SENS_I2C_REGISTER_OUTX_L      0x28
#define ACCEL_SENS_I2C_REGISTER_OUTX_H      0x29
#define ACCEL_SENS_I2C_REGISTER_OUTY_L      0x2A
#define ACCEL_SENS_I2C_REGISTER_OUTY_H      0x2B
#define ACCEL_SENS_I2C_REGISTER_OUTZ_L      0x2C
#define ACCEL_SENS_I2C_REGISTER_OUTZ_H      0x2D
#define ACCEL_SENS_I2C_REGISTER_TEMP_CFG    0x1F
#define ACCEL_SENS_I2C_REGISTER_OUT_TEMP_H  0x0D
#define ACCEL_SENS_I2C_REGISTER_OUT_TEMP_L  0x0C

#define ACCEL_SENS_I2C_TEMPERATURE_OFFSET  (11) /* NOTE: the temperature sensor is not absolute, so there needs to be a offset calibration for each device! */

uint8_t McuLis2dh_GetTemperatureSensorEnabled(bool *enabled) {
  uint8_t res, data = 0;

  res = McuGenericI2C_ReadByteAddress8(ACCEL_SENS_I2C_ADDRESS, ACCEL_SENS_I2C_REGISTER_TEMP_CFG, &data);
  *enabled = data == 0xC0;
  return res;
}

uint8_t McuLis2dh_EnableTemperatureSensor(void) {
  return McuGenericI2C_WriteByteAddress8(ACCEL_SENS_I2C_ADDRESS, ACCEL_SENS_I2C_REGISTER_TEMP_CFG, 0xC0);
}

uint8_t McuLis2dh_DisableTemperatureSensor(void) {
  return McuGenericI2C_WriteByteAddress8(ACCEL_SENS_I2C_ADDRESS, ACCEL_SENS_I2C_REGISTER_TEMP_CFG, 0x00);
}

uint8_t McuLis2dh_WhoAmI(uint8_t *id) {
  /* should be 0b00110011 (0x33) */
  return McuGenericI2C_ReadByteAddress8(ACCEL_SENS_I2C_ADDRESS, ACCEL_SENS_I2C_REGISTER_WHOAMI, id);
}

#if 0
uint8_t McuLis2dh_SetPowerMode(LIS2DH_CTRL_REG1_PowerMode mode) {
  uint8_t val, res;

  res = McuGenericI2C_ReadByteAddress8(ACCEL_SENS_I2C_ADDRESS, ACCEL_SENS_I2C_REGISTER_CTRL_REG1, &val);
  if (res != ERR_OK)
  {
    return res;
  }
  val &= ~LIS2DH_CTRL_REG1_POWERMODE_MASK_BITS; /* mask out existing bits */
  val |= (uint8_t) mode;
  return McuGenericI2C_WriteByteAddress8(ACCEL_SENS_I2C_ADDRESS, ACCEL_SENS_I2C_REGISTER_CTRL_REG1, val);
}
#endif

uint8_t McuLis2dh_Init(void) {
  uint8_t res;

  McuWait_WaitOSms(1);
  res = McuGenericI2C_WriteByteAddress8(ACCEL_SENS_I2C_ADDRESS, ACCEL_SENS_I2C_REGISTER_CTRL_REG1, 0x1F);	//0x1F = 1Hz samples with 16mg/digit & LowpowerMode on, all axes enabled
  res |= McuLis2dh_EnableTemperatureSensor();		//Enable Temp. Measurement
  return res;
}

uint8_t McuLis2dh_getValues(McuLis2dh_AccelAxis_t* values) {
  uint8_t res = 0, temp;
  res |= McuGenericI2C_ReadByteAddress8(ACCEL_SENS_I2C_ADDRESS, ACCEL_SENS_I2C_REGISTER_OUTX_H, (uint8_t *) &values->xValue);
  res |= McuGenericI2C_ReadByteAddress8(ACCEL_SENS_I2C_ADDRESS, ACCEL_SENS_I2C_REGISTER_OUTY_H, (uint8_t *) &values->yValue);
  res |= McuGenericI2C_ReadByteAddress8(ACCEL_SENS_I2C_ADDRESS, ACCEL_SENS_I2C_REGISTER_OUTZ_H, (uint8_t *) &values->zValue);

  res |= McuGenericI2C_WriteByteAddress8(ACCEL_SENS_I2C_ADDRESS, ACCEL_SENS_I2C_REGISTER_CTRL_REG4, 0x80);		//BDU = 1 (needed for Temp.Sensing)
  res |= McuGenericI2C_ReadByteAddress8(ACCEL_SENS_I2C_ADDRESS, ACCEL_SENS_I2C_REGISTER_OUT_TEMP_L, &temp); //Not needed for data acquisition but has to be read out...
  res |= McuGenericI2C_ReadByteAddress8(ACCEL_SENS_I2C_ADDRESS, ACCEL_SENS_I2C_REGISTER_OUT_TEMP_H, &temp);
  res |= McuGenericI2C_WriteByteAddress8(ACCEL_SENS_I2C_ADDRESS, ACCEL_SENS_I2C_REGISTER_CTRL_REG4, 0x00);		//BDU = 1 (needed for Temp.Sensing)
  values->temp = (int8_t) (temp + ACCEL_SENS_I2C_TEMPERATURE_OFFSET); //Tempoffset needed...
  return res;
}

static uint8_t PrintStatus(McuShell_ConstStdIOType *io) {
  McuLis2dh_AccelAxis_t values;
  uint8_t buf[32], res, id;
  bool enabled;

  McuShell_SendStatusStr((unsigned char*) "lis2dh", (const unsigned char*) "\r\n", io->stdOut);

  res = McuLis2dh_WhoAmI(&id);
  if (res == ERR_OK) {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*) "0x");
    McuUtility_strcatNum8Hex(buf, sizeof(buf), id);
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*) "\r\n");
  } else {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*) "ERROR\r\n");
  }
  McuShell_SendStatusStr((unsigned char*) "  WhoAmI", buf, io->stdOut);

  res = McuLis2dh_GetTemperatureSensorEnabled(&enabled);
  if (res == ERR_OK) {
    McuShell_SendStatusStr((unsigned char*) "  Temp enabled", enabled ? (unsigned char*) "yes\r\n" : (unsigned char*) "no\r\n", io->stdOut);
  } else {
    McuShell_SendStatusStr((unsigned char*) "  Temp enabled", (unsigned char*) "ERROR\r\n", io->stdOut);
  }
  McuUtility_Num8sToStr(buf, sizeof(buf), ACCEL_SENS_I2C_TEMPERATURE_OFFSET);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*) "\r\n");
  McuShell_SendStatusStr((unsigned char*) "  Temp offset", buf, io->stdOut);

  res = McuLis2dh_getValues(&values);
  if (res == ERR_OK) {
    McuUtility_Num8sToStr(buf, sizeof(buf), values.temp);
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*) " Degree Celsius\r\n");
  } else {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*) "ERROR\r\n");
  }
  McuShell_SendStatusStr((unsigned char*) "  Temp", buf, io->stdOut);
  if (res == ERR_OK) {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*) "x: ");
    McuUtility_strcatNum8s(buf, sizeof(buf), values.xValue);
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*) ", y: ");
    McuUtility_strcatNum8s(buf, sizeof(buf), values.yValue);
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*) ", z: ");
    McuUtility_strcatNum8s(buf, sizeof(buf), values.zValue);
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*) "\r\n");
  } else {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*) "ERROR\r\n");
  }
  McuShell_SendStatusStr((unsigned char*) "  Accel", buf, io->stdOut);
#if PL_CONFIG_HAS_ACCEL_INT
  McuUtility_strcpy(buf, sizeof(buf), ACC_INT1_GetVal() ? (unsigned char*) "HIGH\r\n" : (unsigned char*) "LOW\r\n");
  McuShell_SendStatusStr((unsigned char*) "  INT1", buf, io->stdOut);
#endif
  return ERR_OK;
}

uint8_t McuLis2dh_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP) == 0 || McuUtility_strcmp((char*)cmd, "LIS2DH help") == 0) {
    McuShell_SendHelpStr((unsigned char*) "lis2dh", (const unsigned char*) "Group of LIS2DH accelerometer commands\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char*) "  help|status", (const unsigned char*) "Print help or status information\r\n", io->stdOut);
    *handled = TRUE;
    return ERR_OK;
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS) == 0) || (McuUtility_strcmp((char*)cmd, "lis2dh status") == 0)) {
    *handled = TRUE;
    return PrintStatus(io);
  }
  return ERR_OK;
}

#endif /* MCU_LIS2DH_CONFIG_IS_ENABLED */
