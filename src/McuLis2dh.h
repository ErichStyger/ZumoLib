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

#ifndef MCU_LIS2DH_ACCELEROMETER_H_
#define MCU_LIS2DH_ACCELEROMETER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "McuLis2dh_config.h"
#if MCU_LIS2DH_CONFIG_IS_ENABLED
#include <stdint.h>
#include <stdbool.h>
#include "McuShell.h"

/*!
 * \brief Parses shell commands for the LIS2DH module.
 * \param cmd Command string to parse.
 * \param handled Set to TRUE if command was handled.
 * \param io Shell standard I/O descriptor.
 * \return Error code, ERR_OK if everything was fine.
 */
uint8_t McuLis2dh_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

typedef enum { /* bits corresponding to register position */
  LIS2DH_CTRL_REG1_POWERMODE_POWERDOWN = 0x00,
  LIS2DH_CTRL_REG1_POWERMODE_1HZ = 0x10,
  LIS2DH_CTRL_REG1_POWERMODE_10HZ = 0x20,
  LIS2DH_CTRL_REG1_POWERMODE_25HZ = 0x30,
  LIS2DH_CTRL_REG1_POWERMODE_50HZ = 0x40,
  LIS2DH_CTRL_REG1_POWERMODE_100HZ = 0x50,
  LIS2DH_CTRL_REG1_POWERMODE_200HZ = 0x60,
  LIS2DH_CTRL_REG1_POWERMODE_400HZ = 0x70,
  LIS2DH_CTRL_REG1_POWERMODE_1620HZ = 0x80,
  LIS2DH_CTRL_REG1_POWERMODE_NORMAL = 0x90,
  LIS2DH_CTRL_REG1_POWERMODE_MASK_BITS = 0xF0,
} LIS2DH_CTRL_REG1_PowerMode;

/*!
 * \brief Sets the LIS2DH output data-rate and power mode bits.
 * \param mode New CTRL_REG1 power mode bits.
 * \return Error code, ERR_OK if everything was fine.
 */
uint8_t McuLis2dh_SetPowerMode(LIS2DH_CTRL_REG1_PowerMode mode);

/*!
 * \brief Writes CTRL_REG1 of the LIS2DH.
 * \param reg1 Register value to write.
 * \return Error code, ERR_OK if everything was fine.
 */
uint8_t McuLis2dh_SetCtrlReg1(uint8_t reg1);

typedef struct {
  int8_t xValue;  /* 1digit = 16mG (8Bit signed) */
  int8_t yValue;
  int8_t zValue;
  int8_t temp;	 /* Temperature in degree C (1 degree C resolution) (8Bit signed) */
} McuLis2dh_AccelAxis_t;

/*!
 * \brief Returns if the internal temperature sensor is enabled.
 * \param enabled Pointer receiving enabled state.
 * \return Error code, ERR_OK if everything was fine.
 */
uint8_t McuLis2dh_GetTemperatureSensorEnabled(bool *enabled);

/*!
 * \brief Enables the internal temperature sensor.
 * \return Error code, ERR_OK if everything was fine.
 */
uint8_t McuLis2dh_EnableTemperatureSensor(void);

/*!
 * \brief Disables the internal temperature sensor.
 * \return Error code, ERR_OK if everything was fine.
 */
uint8_t McuLis2dh_DisableTemperatureSensor(void);

/*!
 * \brief Reads acceleration and temperature values from the sensor.
 * \param values Pointer to value structure to fill.
 * \return Error code, ERR_OK if everything was fine.
 */
uint8_t McuLis2dh_getValues(McuLis2dh_AccelAxis_t* values);

/*!
 * \brief Reads the LIS2DH WHO_AM_I register.
 * \param id Pointer receiving the device ID.
 * \return Error code, ERR_OK if everything was fine.
 */
uint8_t McuLis2dh_WhoAmI(uint8_t *id);

/*!
 * \brief Initializes the LIS2DH module.
 * \return Error code, ERR_OK if everything was fine.
 */
uint8_t McuLis2dh_Init(void);

#endif /* MCU_LIS2DH_CONFIG_IS_ENABLED */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* MCU_LIS2DH_ACCELEROMETER_H_ */
