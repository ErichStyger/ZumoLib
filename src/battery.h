/**
 * \file
 * \brief Module for the battery management.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * Interface to the robot battery management.
 */

#ifndef SOURCES_BATTERY_H_
#define SOURCES_INTRO_ROBOLIBSOURCES_BATTERY_H__BATTERY_H_

#include "platform.h"
#if PL_CONFIG_HAS_BATTERY_ADC
#if PL_CONFIG_USE_SHELL
#include "McuShell.h"

/*!
 * \brief Parses a command
 * \param cmd Command string to be parsed
 * \param handled Sets this variable to TRUE if command was handled
 * \param io I/O stream to be used for input/output
 * \return Error code, ERR_OK if everything was fine
 */
uint8_t BATT_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif /* PL_CONFIG_USE_SHELL */

/*!
 * \brief Does a measurement of the battery voltage
 * \param cvP Pointer to variable where to store the voltage in centi-voltage units (330 is 3.3V)
 * \return Error code, ERR_OK if everything was fine
 */
uint8_t BATT_MeasureBatteryVoltage(uint16_t *cvP);

/*!
 * \brief Module Initialization.
 */
void BATT_Init(void);

/*!
 * \brief Module De-initialization.
 */
void BATT_Deinit(void);

#endif /* PL_CONFIG_HAS_BATTERY_ADC */

#endif /* SOURCES_BATTERY_H_ */
