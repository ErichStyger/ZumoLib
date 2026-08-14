/**
 * \file
 * \brief This is the interface to Remote Controller Module
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ROBOT_TO_ESP_H_
#define __ROBOT_TO_ESP_H_

#include "platform.h"
#if PL_CONFIG_USE_ROBOT2ESP
#include "McuShell.h"

/*!
 * \brief Queues a character received from the ESP side.
 * \param ch Character received from the ESP.
 */
void RobotToEsp_GatewayRxFromESP(unsigned char ch);

/*!
 * \brief Retrieves a character to be forwarded to the ESP.
 * \param pch Destination for the queued character.
 * \return true if a character was retrieved, false otherwise.
 */
bool RobotToEsp_GatewayTxToESP(unsigned char *pch);

/*!
 * \brief Returns the I/O channel used for data received from the ESP.
 * \return I/O handle for ESP RX forwarding.
 */
McuShell_ConstStdIOTypePtr RobotToEsp_GetIOforEspRx(void);

/*!
 * \brief Parses a command
 * \param cmd Command string to be parsed
 * \param handled Sets this variable to TRUE if command was handled
 * \param io I/O stream to be used for input/output
 * \return Error code, ERR_OK if everything was fine
 */
uint8_t RobotToEsp_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*! \brief De-initialization of the module. */
void RobotToEsp_Deinit(void);

/*! \brief Initialization of the module. */
void RobotToEsp_Init(void);

#endif /* PL_CONFIG_USE_ROBOT2ESP */

#endif /* __ROBOT_TO_ESP_H_ */
