/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file
 * \brief Remote controller implementation over WiFi and UDP
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_REMOTE_WIFI_UDP
#include "remoteWifiUdp.h"
#include "McuUdpClient.h"
#include "McuLog.h"
#include "McuUtility.h"

#if McuLib_CONFIG_CPU_IS_ESP32
void RemoteWifiUdp_EspOnButtonEvent(Buttons_e button, McuDbnc_EventKinds event) {
  /* Callback on the remote ESP board, called for button press. Sends a UDP message to the robot. */
  char rxBuf[32];
  const char *host = "esp-n00";
  const int port = 1234;
  char msg[64];

  if (event!=MCUDBNC_EVENT_PRESSED) {
    return;
  }
#if 0
  if (McuUdpClient_Send(host, port, "@esp:esp2robot sendcmd buzzer buz 100 200!", rxBuf, sizeof(rxBuf))!=ERR_OK) {
    McuLog_error("failed sending udp remote message");
  }
#endif
  McuUtility_strcpy((unsigned char*)msg, sizeof(msg), (unsigned char*)"@esp:#esp2robot nav ");
  switch(button) {
    case BUTTONS_NAV_UP:      McuUtility_chcat((unsigned char*)msg, sizeof(msg), 'u'); break;
    case BUTTONS_NAV_DOWN:    McuUtility_chcat((unsigned char*)msg, sizeof(msg), 'd'); break;
    case BUTTONS_NAV_LEFT:    McuUtility_chcat((unsigned char*)msg, sizeof(msg), 'l'); break;
    case BUTTONS_NAV_RIGHT:   McuUtility_chcat((unsigned char*)msg, sizeof(msg), 'r'); break;
    case BUTTONS_NAV_CENTER:  McuUtility_chcat((unsigned char*)msg, sizeof(msg), 'c'); break;
    default: break;
  }
  McuUtility_strcat((unsigned char*)msg, sizeof(msg), (unsigned char*)" on!");
  if (McuUdpClient_Send(host, port, msg, rxBuf, sizeof(rxBuf))!=ERR_OK) {
    McuLog_error("failed sending udp remote nav message '%s'", msg);
  } else {
    McuLog_info("sent nav message `%s`, response `%s`", msg, rxBuf);
  }
}
#endif

#endif /* PL_CONFIG_USE_REMOTE_WIFI_UDP */