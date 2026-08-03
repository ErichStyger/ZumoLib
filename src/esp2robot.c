/*
 * Copyright (c) 2021-2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_ESP2ROBOT
#include "McuShell.h"
#include "McuUtility.h"
#include "shell.h"

#if PL_CONFIG_USE_SHELL
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  McuShell_SendStatusStr((unsigned char*)"robo", (unsigned char*)"ESP32 robo status\r\n", io->stdOut);
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"esp2robot", (unsigned char*)"Group of ESP to Robot commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows esp2robot help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  send <text>", (unsigned char*)"Send a text to the robot\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  sendcmd <cmd>", (unsigned char*)"Send a command to the robot, e.g. '#buzzer buz 100 200'\r\n", io->stdOut);
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
    Shell_SendToRobotAndGetResponse(p, response, sizeof(response));
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
