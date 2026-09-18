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
#include "remoteWifiUdp_config.h"
#include "remoteWifiUdp.h"
#include "McuUdpClient.h"
#include "McuLog.h"
#include "McuUtility.h"
#include "minIni/McuMinINI.h"

static struct settings_ {
  bool isEnabled;
  char destination_host[48];
  uint16_t destination_port;
} settings;

uint16_t GetDefaultHostPort(void) {
  return settings.destination_port;
}

static uint8_t SetDefaultHostPort(uint16_t port) {
  settings.destination_port = port;
  if (McuMinINI_ini_putl(REMOTE_WIFI_UDP_CONFIG_MININI_SECTION_NAME, REMOTE_WIFI_UDP_CONFIG_MININI_KEY_PORT, port, REMOTE_WIFI_UDP_CONFIG_MININI_FILE_NAME)!=1) { /* 1: success */
    return ERR_FAILED;
  }
  return ERR_OK;
}

static const char *GetDefaultHostName(void) {
  return settings.destination_host;
}

uint8_t SetDefaultHostName(const char *host) {
  McuUtility_strcpy((uint8_t*)settings.destination_host, sizeof(settings.destination_host), (const unsigned char*)host);
  return ERR_OK;
}

static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  unsigned char buf[64];

  McuShell_SendStatusStr((unsigned char*)"remotewifiudp", (unsigned char*)"Remote WiFi UDP status\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  enabled", settings.isEnabled?(const unsigned char*)"yes\r\n":(const unsigned char*)"no\r\n", io->stdOut);
  McuUtility_strcpy(buf, sizeof(buf), (const unsigned char*)GetDefaultHostName());
  McuUtility_chcat(buf, sizeof(buf), ':');
  McuUtility_strcatNum16u(buf, sizeof(buf), GetDefaultHostPort());
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  host:port", buf, io->stdOut);
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"remotewifiudp", (unsigned char*)"Group of Remote WiFi UDP client commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  host <string>", (unsigned char*)"Set default host destination IP address or host name\r\n", io->stdOut);
  return ERR_OK;
}

uint8_t RemoteWifiUdp_ParseCommand(const unsigned char* cmd, bool *handled, const McuShell_StdIOType *io) {
  const unsigned char *p;

  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"remotewifiudp help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"remotewifiudp status")==0) {
    *handled = TRUE;
    return PrintStatus(io);
  } else if (McuUtility_strncmp((char*)cmd, (char*)"remotewifiudp host ", sizeof("remotewifiudp host ")-1)==0) {
    *handled = TRUE;
    p = cmd + sizeof("udpc host ")-1;
    SetDefaultHostName((const char*)p);
    return ERR_OK;
  } else if (McuUtility_strncmp((char*)cmd, (char*)"remotewifiudp port ", sizeof("remotewifiudp port ")-1)==0) {
    uint16_t port;
    *handled = TRUE;
    p = cmd + sizeof("remotewifiudp port ")-1;
    if (McuUtility_ScanDecimal16uNumber(&p, &port)!=ERR_OK) {
      return ERR_FAILED;
    }
    return SetDefaultHostPort(port);
  }
  return ERR_OK;
}

void RemoteWifiUdp_LoadSettings(void) {
  settings.isEnabled = McuMinINI_ini_getbool(REMOTE_WIFI_UDP_CONFIG_MININI_SECTION_NAME, REMOTE_WIFI_UDP_CONFIG_MININI_KEY_ENABLED, false, REMOTE_WIFI_UDP_CONFIG_MININI_FILE_NAME);
  McuMinINI_ini_gets(REMOTE_WIFI_UDP_CONFIG_MININI_SECTION_NAME, REMOTE_WIFI_UDP_CONFIG_MININI_KEY_HOST, REMOTE_WIFI_UDP_CONFIG_HOST_NAME, settings.destination_host, sizeof(settings.destination_host), REMOTE_WIFI_UDP_CONFIG_MININI_FILE_NAME);
  settings.destination_port = McuMinINI_ini_getbool(REMOTE_WIFI_UDP_CONFIG_MININI_SECTION_NAME, REMOTE_WIFI_UDP_CONFIG_MININI_KEY_PORT, REMOTE_WIFI_UDP_CONFIG_HOST_PORT, REMOTE_WIFI_UDP_CONFIG_MININI_FILE_NAME);
}

void RemoteWifiUdp_EspOnButtonEvent(Buttons_e button, McuDbnc_EventKinds event) {
  /* Callback on the remote ESP board, called for button press. Sends a UDP message to the robot. */
  char rxBuf[32];
  const char *host = REMOTE_WIFI_UDP_CONFIG_HOST_NAME; /* \todo needs to be configurable */
  const int port = REMOTE_WIFI_UDP_CONFIG_HOST_PORT;
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

#endif /* PL_CONFIG_USE_REMOTE_WIFI_UDP */
