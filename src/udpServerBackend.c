/**
 * \file
 * \brief Backend for UDP messages received from the server.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "udpServerBackend.h"
#include "McuUdpServer.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "shell.h"

static void UdpIncomingCallback(const char *rxBuffer, int rxLen, char *responseBuf, size_t responseBufLen, size_t *responseSize) {
  char shellMsg[McuShell_DEFAULT_SHELL_BUFFER_SIZE]; /* buffer for message */
  #define MSG_ESP_PREFIX_STR   "@esp:"

  if (rxBuffer[rxLen] == '\0') { /* zero terminated? */
    McuLog_info("handling incoming udp message string: '%s'", rxBuffer);
  } else { /* not a string? */
    McuLog_info("handling incoming udp message: rxLen %d", rxLen);
  }
  McuUtility_strcpy((unsigned char*)responseBuf, responseBufLen, (unsigned char*)"OK"); /* set a default response */
  /* check message and framing */
  if (McuUtility_strncmp(rxBuffer, MSG_ESP_PREFIX_STR, sizeof(MSG_ESP_PREFIX_STR)-1)==0) { /* check prefix */
    size_t strLen = McuUtility_strlen(rxBuffer);
    if (rxBuffer[strLen-1]=='!') { /* matching end framing? */
      /* send to ESP32 shell */
      McuUtility_strcpy((unsigned char*)shellMsg, sizeof(shellMsg), (unsigned char*)(rxBuffer+strlen(MSG_ESP_PREFIX_STR))); /* copy the command after the prefix */
      shellMsg[McuUtility_strlen((char*)shellMsg)-1] = '\0'; /* replace '!' at the end */
    //  SHELL_SendToESPAndGetResponse(msg, responseBuf, responseBufLen);
    } else {
      McuUtility_strcpy((unsigned char*)responseBuf, responseBufLen, (unsigned char*)"'!' missing!");
    }
  }
  *responseSize = McuUtility_strlen(responseBuf)+1; /* +1: include zero byte */
}

void UdpServerBackend_Init(void) {
  McuUdpServer_SetIncomingCallback(UdpIncomingCallback);
}