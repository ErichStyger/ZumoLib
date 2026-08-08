/*
 * Copyright (c) 2021-2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * \brief This module is for the ESP2 and responsible for the ESP to send commands to the robot.
 */

#include "platform.h"
#if PL_CONFIG_USE_ESP2ROBOT
#include "McuShell.h"
#include "McuUtility.h"
#include "McuRTOS.h"
#include "McuShellUart.h"
#include "shell.h"

static void SendToRobotAndGetResponse(const unsigned char *send, unsigned char *response, size_t responseSize) {
  unsigned char buffer[128]; /* buffer for sending command to robot */

  /* build a frame around the message: that way the robot is able to recognize it */
  McuUtility_strcpy(buffer, sizeof(buffer), (unsigned char*)"@robot:cmd ");
  McuUtility_strcat(buffer, sizeof(buffer), send);
  McuUtility_strcat(buffer, sizeof(buffer), (unsigned char*)"!\r\n");
  Shell_SendString(buffer); /* send to UART, which is read by the robot */
  /* get response */
#if 1
  /* Important: this consumes directly all characters coming from the robot. That way the ESP32 shell does not get it.
   * A mutex is used to block the shell from getting the UART stream.
   */
  #define TIMEOUT_MS  (500) /* stop if we don't get new input after this timeout */
  int timeoutMs = TIMEOUT_MS;

  *response = '\0';
  if (xSemaphoreTakeRecursive(Shell_GetMutex(), portMAX_DELAY)==pdPASS) { /* take mutex */
    while (true) { /* breaks after timeout */
      if (!McuShellUart_stdio.keyPressed()) { /* no input: wait for timeout */
        timeoutMs -= 50;
        if (timeoutMs<=0) {
          break; /* timeout */
        }
        vTaskDelay(pdMS_TO_TICKS(50));
      } else { /* character available */
        unsigned char ch;
        McuShellUart_stdio.stdIn(&ch);
        if (ch!='\r') { /* filter out '\r' in "\r\n" */
          McuUtility_chcat(response, responseSize, ch);
        }
        timeoutMs = TIMEOUT_MS; /* reset timeout */
      } /* if */
    } /* while */
    (void)xSemaphoreGiveRecursive(Shell_GetMutex()); /* give back mutex */
  }
  if (*response=='\0') { /* if response is empty, send back at least an acknowledgment */
    McuUtility_strcpy(response, responseSize, (unsigned char*)"OK"); /* default response */
  }
#else
  McuUtility_strcpy(response, responseSize, (unsigned char*)"OK"); /* default response */
#endif
}

#if PL_CONFIG_USE_SHELL
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  McuShell_SendStatusStr((unsigned char*)"esp2robot", (unsigned char*)"ESP-2-Robot channel status\r\n", io->stdOut);
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"esp2robot", (unsigned char*)"Group of ESP to Robot commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows esp2robot help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  send <text>", (unsigned char*)"Send a text to the robot\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  sendcmd <cmd>", (unsigned char*)"Send a command to the robot, e.g. \"#buzzer buz 100 200\"\r\n", io->stdOut);
  return ERR_OK;
}

uint8_t Esp2robot_ParseCommand(const unsigned char* cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"esp2robot help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"esp2robot status")==0) {
    *handled = TRUE;
    return PrintStatus(io);
  } else if (McuUtility_strncmp((char*)cmd, (char*)"esp2robot send ", sizeof("esp2robot send ")-1)==0) {
    const unsigned char *p;

    *handled = TRUE;
    p = cmd+sizeof("esp2robot send ")-1;
    McuShell_SendStr(p, io->stdOut); /* send to standard I/O which is the UART to the robot */
    return ERR_OK;
  } else if (McuUtility_strncmp((char*)cmd, (char*)"esp2robot sendcmd ", sizeof("esp2robot sendcmd ")-1)==0) {
    static uint8_t response[10*1024];
    unsigned char buffer[McuShell_CONFIG_DEFAULT_SHELL_BUFFER_SIZE];
    const unsigned char *p;

    *handled = TRUE;
    p = cmd+sizeof("esp2robot sendcmd ")-1;
    while (*p==' ') { /* skip leading spaces */
      p++;
    }
    if (*p=='"') { /* double-quoted command: it can contain multiple commands */
      if (McuUtility_ScanDoubleQuotedString(&p, buffer, sizeof(buffer))!=ERR_OK) {
        return ERR_FAILED;
      }
      p = buffer;
    }
    SendToRobotAndGetResponse(p, response, sizeof(response));
    McuShell_SendStr(response, io->stdOut); /* show result on console */
    return ERR_OK;
  }
  return ERR_OK;
}
#endif /* PL_CONFIG_USE_SHELL */

void Esp2robot_Deinit(void) {
  /* nothing needed */
}

void Esp2robot_Init(void) {
  /* nothing needed */
}
#endif /* PL_CONFIG_USE_ESP2ROBOT */
