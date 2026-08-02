/**
 * \file
 * \brief Interface for remote controller using Nordic NRF24L01+
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCES_Remote_Nordic_H_
#define SOURCES_Remote_Nordic_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"

#if PL_CONFIG_USE_REMOTE_NORDIC
  #include "McuLib.h"
  #include "McuDebounce.h"

  #if PL_CONFIG_USE_SHELL
  #include "McuShell.h"

  uint8_t RemoteNordic_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
  #endif

  #include "RNet_App.h"

  uint8_t RemoteNordic_HandleRemoteRxMessage(RAPP_MSG_Type type, uint8_t size, uint8_t *data, RNWK_ShortAddrType srcAddr, bool *handled, RPHY_PacketDesc *packet);

  #if McuLib_CONFIG_CPU_IS_ESP32
    #include "buttons.h"
    void RemoteNordic_ESP32OnButtonEvent(Buttons_e button, McuDbnc_EventKinds event);
  #endif

  typedef enum RemoteNordic_RobotMoveStatus_e {
    RemoteNordic_MOVE_STATUS_UNKNOWN=0,
    RemoteNordic_MOVE_STATUS_STOPPED=1,
    RemoteNordic_MOVE_STATUS_MOVING=2,
  } RemoteNordic_RobotMoveStatus_e;

  RemoteNordic_RobotMoveStatus_e RemoteNordic_GetRobotMoveStatus(void);
  void RemoteNordic_SetRobotMoveStatus(RemoteNordic_RobotMoveStatus_e status);

  int RemoteNordic_QueryRobotBatteryVoltage(void);
  uint32_t RemoteNordic_GetRobotBatteryVoltage_mV(void);

  void RemoteNordic_HandleIncomingUdpMessage(const char *rxMsg, const char *responseBuf, size_t responseBufSize);

  void RemoteNordic_Init(void);

#endif /* PL_CONFIG_USE_REMOTE_NORDIC */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* SOURCES_Remote_Nordic_H_ */
