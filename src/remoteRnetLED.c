/**
 * \file
 * \brief Interface for the Remote RNet LED module.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCES_REMOTE_RNET_LED_H_
#define SOURCES_REMOTE_RNET_LED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"
#include "McuShell.h"
#include "RNet_App.h"

/*!
 * \brief Parses a shell command.
 * \param cmd Command string to parse.
 * \param handled Set to true if the command was handled.
 * \param io Shell I/O handler.
 * \return Error code, or ERR_OK.
 */
uint8_t RemoteRnetLED_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*!
 * \brief Handles an incoming RNet message.
 * \param type Message type.
 * \param size Message payload size.
 * \param data Message payload.
 * \param srcAddr Source address.
 * \param handled Set to true if the message was handled.
 * \param packet Packet descriptor.
 * \return Error code, or ERR_OK.
 */
uint8_t RemoteRnetLED_HandleRemoteRxMessage(RAPP_MSG_Type type, uint8_t size, uint8_t *data, RNWK_ShortAddrType srcAddr, bool *handled, RPHY_PacketDesc *packet);

/*!
 * \brief Initializes the RemoteRnetLED module.
 */
void RemoteRnetLED_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* SOURCES_REMOTE_RNET_LED_H_ */
