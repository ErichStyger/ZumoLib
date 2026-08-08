/**
 * \file
 * \brief Interface for the RoboLED module
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCES_ROBO_LED_H_
#define SOURCES_ROBO_LED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"
#include "McuShell.h"
#include "RNet_App.h"

uint8_t RoboLED_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

uint8_t RoboLED_HandleRemoteRxMessage(RAPP_MSG_Type type, uint8_t size, uint8_t *data, RNWK_ShortAddrType srcAddr, bool *handled, RPHY_PacketDesc *packet);

void RobotLED_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* SOURCES_ROBO_LED_H_ */
