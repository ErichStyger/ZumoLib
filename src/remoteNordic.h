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

  /*!
   * \brief Parses a shell command.
   * \param cmd Command string to parse.
   * \param handled Set to true if the command was handled.
   * \param io Shell I/O handler.
   * \return Error code, or ERR_OK.
   */
  uint8_t RemoteNordic_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
  #endif

  #include "RNet_App.h"

  /*!
   * \brief Handles an incoming radio message.
   * \param type Message type.
   * \param size Message payload size.
   * \param data Message payload.
   * \param srcAddr Source address.
   * \param handled Set to true if the message was handled.
   * \param packet Packet descriptor.
   * \return Error code, or ERR_OK.
   */
  uint8_t RemoteNordic_HandleRemoteRxMessage(RAPP_MSG_Type type, uint8_t size, uint8_t *data, RNWK_ShortAddrType srcAddr, bool *handled, RPHY_PacketDesc *packet);

  #if McuLib_CONFIG_CPU_IS_ESP32
    #include "buttons.h"
    /*!
     * \brief Forwards an ESP32 button event to the Nordic remote handling.
     * \param button Button that triggered the event.
     * \param event Debounce event kind.
     */
    void RemoteNordic_EspOnButtonEvent(Buttons_e button, McuDbnc_EventKinds event);
  #endif

  typedef enum RemoteNordic_RobotMoveStatus_e {
    RemoteNordic_MOVE_STATUS_UNKNOWN=0,
    RemoteNordic_MOVE_STATUS_STOPPED=1,
    RemoteNordic_MOVE_STATUS_MOVING=2,
  } RemoteNordic_RobotMoveStatus_e;

  /*!
   * \brief Returns the current robot move status.
   * \return Current move status.
   */
  RemoteNordic_RobotMoveStatus_e RemoteNordic_GetRobotMoveStatus(void);

  /*!
   * \brief Updates the robot move status.
   * \param status New move status.
   */
  void RemoteNordic_SetRobotMoveStatus(RemoteNordic_RobotMoveStatus_e status);

  /*!
   * \brief Queries the robot battery voltage.
   * \return Error code, or ERR_OK.
   */
  int RemoteNordic_QueryRobotBatteryVoltage(void);

  /*!
   * \brief Returns the last measured robot battery voltage.
   * \return Battery voltage in mV.
   */
  uint32_t RemoteNordic_GetRobotBatteryVoltage_mV(void);

  /*!
   * \brief Handles an incoming UDP message.
   * \param rxMsg Received message text.
   * \param responseBuf Buffer to write a response into.
   * \param responseBufSize Size of the response buffer.
   */
  void RemoteNordic_HandleIncomingUdpMessage(const char *rxMsg, const char *responseBuf, size_t responseBufSize);

  /*!
   * \brief Initializes the Nordic remote module.
   */
  void RemoteNordic_Init(void);

#endif /* PL_CONFIG_USE_REMOTE_NORDIC */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* SOURCES_Remote_Nordic_H_ */
