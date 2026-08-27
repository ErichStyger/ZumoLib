/**
 * \file
 * \brief Backend for UDP messages received from the server.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_UDP_SERVER_BACKEND
#include "udpServerBackend.h"
#include "McuUdpServer.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "shell.h"

static char *esp_io_buf; /* pointer to buffer */
static size_t esp_io_buf_size; /* size of buffer */

static void esp_io_buf_SendChar(unsigned char ch) {
  McuUtility_chcat((unsigned char*)esp_io_buf, esp_io_buf_size, ch);
}

static void esp_io_buf_ReadChar(uint8_t *c) {
  *c = '\0';
}

static bool esp_io_buf_CharPresent(void) {
  return false;
}

static McuShell_ConstStdIOType esp_stdio = {
  .stdIn = (McuShell_StdIO_In_FctType)esp_io_buf_ReadChar,
  .stdOut = (McuShell_StdIO_OutErr_FctType)esp_io_buf_SendChar,
  .stdErr = (McuShell_StdIO_OutErr_FctType)esp_io_buf_SendChar,
  .keyPressed = esp_io_buf_CharPresent, /* if input is not empty */
#if McuShell_CONFIG_ECHO_ENABLED
  .echoEnabled = false,
#endif
};

static void sendToESPAndGetResponse(const char *msg, char *response, size_t responseSize) {
  esp_io_buf = response;
  esp_io_buf_size = responseSize;
  esp_io_buf[0] = '\0'; /* initialize buffer */
  McuLog_info("Sending to ESP Shell: %s", msg);
  Shell_ParseCommandIO((unsigned char*)msg, &esp_stdio, true); /* send to ESP32 shell */
  if (response[0]=='\0') { /* empty response? add a default */
    McuUtility_strcpy((unsigned char*)response, responseSize, (unsigned char*)"OK"); /* default response */
  }
}

static void UdpIncomingCallback(const char *rxBuffer, int rxLen, char *responseBuf, size_t responseBufLen, size_t *responseSize) {
  char shellMsg[McuShell_DEFAULT_SHELL_BUFFER_SIZE]; /* buffer for message */
  #define MSG_ESP_PREFIX_STR   "@esp:"

  if (rxBuffer[rxLen] == '\0') { /* zero terminated? */
    McuLog_info("handling incoming udp message string: rxLen %d, \"%s\"", rxLen, rxBuffer);
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
      sendToESPAndGetResponse(shellMsg, responseBuf, responseBufLen);
    } else {
      McuUtility_strcpy((unsigned char*)responseBuf, responseBufLen, (unsigned char*)"'!' missing!");
    }
  }
  *responseSize = McuUtility_strlen(responseBuf)+1; /* +1: include zero byte */
}

void UdpServerBackend_Init(void) {
  McuUdpServer_SetIncomingCallback(UdpIncomingCallback);
}

#endif /* PL_CONFIG_USE_UDP_SERVER_BACKEND */
