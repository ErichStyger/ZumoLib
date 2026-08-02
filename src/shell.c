/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief Shell module implementation.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_SHELL
#include "shell.h"
#include "McuShell.h"
#include "McuRTOS.h"
#include "McuRTT.h"
#include "McuArmTools.h"
#include "McuLog.h"
#include "McuShellUart.h"
#include "McuTimeDate.h"
#if PL_CONFIG_USE_BLINKY
  #include "blinky.h"
#endif
#if PL_CONFIG_USE_MOTORS
  #include "motor.h"
#endif
#if PL_CONFIG_USE_QUADRATURE
  #include "quadCounter.h"
#endif
#if PL_CONFIG_USE_TACHO
  #include "tacho.h"
#endif
#if PL_CONFIG_USE_PID
  #include "pid.h"
#endif
#if PL_CONFIG_USE_DRIVE
  #include "drive.h"
#endif
#if PL_CONFIG_USE_TURN
  #include "turn.h"
#endif
#if PL_CONFIG_USE_BUZZER
  #include "buzzer.h"
#endif
#if PL_CONFIG_USE_REFLECTANCE
  #include "reflectance.h"
#endif
#if McuFlash_CONFIG_IS_ENABLED
  #include "McuFlash.h"
#endif
#if PL_CONFIG_USE_MININI
  #include "minIni/McuMinINI.h"
#endif
#if PL_CONFIG_USE_SHELL_CDC
  #include "McuShellCdcDevice.h"
#endif
#if PL_CONFIG_USE_ESP32
  #include "McuESP32.h"
#endif
#if MCU_LIS2DH_CONFIG_IS_ENABLED
  #include "McuLis2dh.h"
#endif
#if PL_CONFIG_LINE_FOLLOWING
  #include "lineFollow.h"
#endif
#if PL_CONFIG_USE_IDENTIFY
  #include "identify.h"
#endif
#if PL_CONFIG_USE_NORDIC_RADIO
  #include "RNet/McuRNet.h"
  #include "RNet/RStdIO.h"
  #include "RNet_App.h"
#endif
#if PL_CONFIG_USE_REMOTE_NORDIC
  #include "remoteNordic.h"
#endif
#if PL_CONFIG_USE_LEDS
  #include "leds.h"
#endif

typedef struct {
  McuShell_ConstStdIOType *stdio;
  unsigned char *buf;
  size_t bufSize;
} SHELL_IODesc;

static const SHELL_IODesc ios[] =
{
#if PL_CONFIG_USE_SHELL_UART
  {&McuShellUart_stdio,  McuShellUart_DefaultShellBuffer,  sizeof(McuShellUart_DefaultShellBuffer)},
#endif
#if PL_CONFIG_USE_SHELL_CDC
  {&McuShellCdcDevice_stdio,  McuShellCdcDevice_DefaultShellBuffer,  sizeof(McuShellCdcDevice_DefaultShellBuffer)},
#endif
#if PL_CONFIG_USE_SHELL_RTT
  {&McuRTT_stdio,  McuRTT_DefaultShellBuffer,  sizeof(McuRTT_DefaultShellBuffer)},
#endif
#if PL_CONFIG_USE_NORDIC_RADIO && RNET_CONFIG_REMOTE_STDIO
  {&RSTDIO_stdio, RSTDIO_DefaultShellBuffer, sizeof(RSTDIO_DefaultShellBuffer)},
#endif
};

static const McuShell_ParseCommandCallback CmdParserTable[] =
{
  McuShell_ParseCommand, /* McuShell component, is first in list */
  McuRTOS_ParseCommand, /* FreeRTOS shell parser */
  McuArmTools_ParseCommand,
#if PL_CONFIG_USE_LEDS
  Leds_ParseCommand,
#endif
#if PL_CONFIG_USE_BLINKY
  Blinky_ParseCommand,
#endif
#if McuLog_CONFIG_IS_ENABLED
  McuLog_ParseCommand,
#endif
#if PL_CONFIG_USE_IDENTIFY
  ID_ParseCommand,
#endif
#if McuFlash_CONFIG_IS_ENABLED
  McuFlash_ParseCommand,
#endif
#if PL_CONFIG_USE_MOTORS
  Motor_ParseCommand,
#endif
#if PL_CONFIG_USE_QUADRATURE
  QuadCounter_ParseCommand,
#endif
#if PL_CONFIG_USE_TACHO
  Tacho_ParseCommand,
#endif
#if PL_CONFIG_USE_PID
  PID_ParseCommand,
#endif
#if PL_CONFIG_USE_DRIVE
  DRV_ParseCommand,
#endif
#if PL_CONFIG_USE_TURN
  TURN_ParseCommand,
#endif
#if PL_CONFIG_USE_BUZZER
  Buzzer_ParseCommand,
#endif
#if PL_CONFIG_USE_REFLECTANCE
  REF_ParseCommand,
#endif
#if PL_CONFIG_LINE_FOLLOWING
  LineFollow_ParseCommand,
#endif
#if PL_CONFIG_USE_MININI
  McuMinINI_ParseCommand,
  ini_ParseCommand,
#endif
#if MCU_LIS2DH_CONFIG_IS_ENABLED
  McuLis2dh_ParseCommand,
#endif
#if PL_CONFIG_USE_SHELL_CDC
  McuShellCdcDevice_ParseCommand,
#endif
#if PL_CONFIG_USE_ESP32
  McuESP32_ParseCommand,
#endif
#if PL_CONFIG_USE_NORDIC_RADIO
  McuRNet_ParseCommand,
  RNETA_ParseCommand,
#endif
#if PL_CONFIG_USE_REMOTE_NORDIC
  RemoteNordic_ParseCommand,
#endif
  NULL /* Sentinel */
};

static void ShellTask(void *pvParameters) {
  int i;

  (void)pvParameters; /* not used */
  McuLog_trace("started shell task");
  /* initialize buffers */
  for(i=0;i<sizeof(ios)/sizeof(ios[0]);i++) {
    ios[i].buf[0] = '\0';
  }
  for(;;) {
    /* process all I/Os */
    for(i=0;i<sizeof(ios)/sizeof(ios[0]);i++) {
      (void)McuShell_ReadAndParseWithCommandTable(ios[i].buf, ios[i].bufSize, ios[i].stdio, CmdParserTable);
    }
  #if PL_CONFIG_USE_NORDIC_RADIO && RNET_CONFIG_REMOTE_STDIO
    RSTDIO_Print(McuShell_GetStdio()); /* dispatch incoming messages */
  #endif
    vTaskDelay(pdMS_TO_TICKS(10));
  } /* for */
}

#if McuESP32_CONFIG_IS_ENABLED
/* write output from the ESP to the shell too */
static void ESP_SendChar(unsigned char ch) {
#if PL_CONFIG_USE_SHELL_UART
  McuShellUart_stdio.stdOut(ch);
#endif
}

static void ESP_ReadChar(uint8_t *c) {
  *c = '\0'; /* nothing received */
}

static bool ESP_CharPresent(void) {
  return false;
}

McuShell_ConstStdIOType ESP_ToShellStdio = {
    .stdIn = (McuShell_StdIO_In_FctType)ESP_ReadChar,
    .stdOut = (McuShell_StdIO_OutErr_FctType)ESP_SendChar,
    .stdErr = (McuShell_StdIO_OutErr_FctType)ESP_SendChar,
    .keyPressed = ESP_CharPresent, /* if input is not empty */
  #if McuShell_CONFIG_ECHO_ENABLED
    .echoEnabled = false,
  #endif
  };
#endif /* McuESP32_CONFIG_IS_ENABLED */

void Shell_Init(void) {
  if (xTaskCreate(ShellTask, "Shell", (2*1024)/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) {
    for(;;){} /* error */
  }
  McuShell_SetStdio(ios[0].stdio);
}

void Shell_Deinit(void) {
  McuShell_SetStdio(NULL);
}

#endif /* PL_CONFIG_USE_SHELL */
