/**
 * \file
 * \brief This is the interface to Remote Controller Module
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ROBOT_TO_ESP_H_
#define __ROBOT_TO_ESP_H_

#include "platform.h"
#if PL_CONFIG_USE_ROBOT_TO_ESP
#include "McuShell.h"


/* called by the gateway task to put a char from the ESP into the queue for the remote */
void RobotToEsp_GatewayRxFromESP(unsigned char ch);

/* called by the gateway task to get chars from the remote task to be sent to the ESP32 */
bool RobotToEsp_GatewayTxToESP(unsigned char *pch);

/* return the I/O to be used for forwarding data received from the ESP */
McuShell_ConstStdIOTypePtr RobotToEsp_GetIOforEspRx(void);

/*!
 * \brief Parses a command
 * \param cmd Command string to be parsed
 * \param handled Sets this variable to TRUE if command was handled
 * \param io I/O stream to be used for input/output
 * \return Error code, ERR_OK if everything was fine
 */
uint8_t RobotToEsp_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*! \brief De-initialization of the module */
void RobotToEsp_Deinit(void);

/*! \brief Initialization of the module */
void RobotToEsp_Init(void);

#endif /* PL_CONFIG_USE_ROBOT_TO_ESP */

#endif /* __ROBOT_TO_ESP_H_ */
