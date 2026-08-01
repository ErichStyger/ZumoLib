/**
 * \file
 * \brief This is the interface to Tachometer Module
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __Tacho_H_
#define __Tacho_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"
#include <stdbool.h>
#include <stdint.h>

#if PL_CONFIG_USE_TACHO

#define TACHO_SAMPLE_PERIOD_MS (5)     
  /*!< speed sample period in ms. Make sure that speed is sampled at the given rate. */
  
/*!
 * \brief Returns the previously calculated speed of the motor.
 * \param isLeft TRUE for left speed, FALSE for right speed.
 * \return Actual speed value
 */
int32_t Tacho_GetSpeed(bool isLeft);

/*!
 * \brief Calculates the speed based on the position information from the encoder.
 */
void Tacho_CalcSpeed(void);

/*! 
 * \brief Start tacho sampling timer.
 */
void Tacho_StartSamplingTimer(void);

/*! 
 * \brief Stop tacho sampling timer.
 */
void Tacho_StopSamplingTimer(void);

#if PL_CONFIG_USE_SHELL
#include "McuShell.h"
/*!
 * \brief Parses a command
 * \param cmd Command string to be parsed
 * \param handled Sets this variable to TRUE if command was handled
 * \param io I/O stream to be used for input/output
 * \return Error code, ERR_OK if everything was fine
 */
uint8_t Tacho_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif

/*! \brief De-initialization of the module */
void Tacho_Deinit(void);

/*! \brief Initialization of the module */
void Tacho_Init(void);

#endif /* PL_CONFIG_USE_TACHO */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __Tacho_H_ */
