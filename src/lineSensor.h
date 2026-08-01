#if 0
/**
 * \file
 * \brief Line sensor driver interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * This module implements a driver for the robot front IR line sensors.
 */

#ifndef LINE_SENSOR_H_
#define LINE_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lineSensor_config.h"

#include "McuShell.h"
/*!
 * \brief Shell command line parser.
 * \param[in] cmd Pointer to command string
 * \param[out] handled If command is handled by the parser
 * \param[in] io Std I/O handler of shell
 * \return Error code, ERR_OK if everything was fine.
 */
uint8_t LineSensor_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*!
 * \brief Initialization function.
 */
void LineSensor_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* LINE_SENSOR_H_ */

#endif