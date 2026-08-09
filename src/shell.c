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
#if PL_CONFIG_USE_RTT
  #include "McuRTT.h"
#endif
#if McuLib_CONFIG_CPU_IS_ARM_CORTEX_M
  #include "McuArmTools.h"
#endif
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

/* ESP specific parts: */
#if PL_CONFIG_USE_WIFI
  #include "McuWiFi.h"
#endif
#if PL_CONFIG_USE_PING
  #include "McuPingShell.h"
#endif
#if PL_CONFIG_USE_UDP_SERVER
  #include "McuUdpServerShell.h"
#endif
#if PL_CONFIG_USE_UDP_CLIENT
  #include "McuUdpClientShell.h"
#endif
#if PL_CONFIG_USE_MQTT_CLIENT
  #include "McuMqttClient.h"
#endif
#if PL_CONFIG_USE_NTP_CLIENT
  #include "McuNtpClient.h"
#endif
#if PL_CONFIG_USE_SENSIRION
  #include "sensirion.h"
#endif
#if PL_CONFIG_USE_SENSIRION && PL_CONFIG_USE_SHT31
  #include "McuSHT31.h"
#elif PL_CONFIG_USE_SENSIRION && PL_CONFIG_USE_SHT40
  #include "McuSHT40.h"
#endif
#if PL_CONFIG_USE_RS485
  #include "rs485.h"
  #include "McuUart485.h"
#endif
#if McuLib_CONFIG_CPU_IS_ESP32
  #include "driver/uart.h"
  #include "driver/gpio.h" /* \TODO */
#endif
#if PL_CONFIG_USE_ESP2ROBOT
  #include "esp2robot.h"
#endif
#if PL_CONFIG_USE_REMOTE_RNET_LED
  #include "remoteRnetLED.h"
#endif
#if PL_CONFIG_USE_ROBOT_TO_ESP
  #include "robotToEsp.h"
#endif

static const McuShell_ParseCommandCallback CmdParserTable[] =
{
  McuShell_ParseCommand, /* McuShell component, is first in list */
  McuRTOS_ParseCommand, /* FreeRTOS shell parser */
#if McuLib_CONFIG_CPU_IS_ARM_CORTEX_M
  McuArmTools_ParseCommand,
#endif
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
#if PL_CONFIG_USE_WIFI
  McuWiFi_ParseCommand,
#endif
#if PL_CONFIG_USE_UDP_CLIENT
  McuUdpClient_ParseCommand,
#endif
#if PL_CONFIG_USE_UDP_SERVER
  McuUdpServer_ParseCommand,
#endif
#if PL_CONFIG_USE_PING
  McuPing_ParseCommand,
#endif
#if PL_CONFIG_USE_MQTT_CLIENT
  McuMqttClient_ParseCommand,
#endif
#if PL_CONFIG_USE_NTP_CLIENT
  McuNtpClient_ParseCommand,
#endif
#if PL_CONFIG_USE_SENSIRION && PL_CONFIG_USE_SHT31
  McuSHT31_ParseCommand,
#elif PL_CONFIG_USE_SENSIRION && PL_CONFIG_USE_SHT40
  McuSHT40_ParseCommand,
#endif
#if PL_CONFIG_USE_RS485 && McuUart485_CONFIG_USE_RS_485
  McuUart485_ParseCommand,
#endif
#if PL_CONFIG_USE_RS485 && PL_CONFIG_USE_RS485_SHELL && !McuUart485_CONFIG_USE_RAW
  RS485_ParseCommand,
#endif
#if PL_CONFIG_USE_ESP2ROBOT
  Esp2robot_ParseCommand,
#endif
#if PL_CONFIG_USE_REMOTE_RNET_LED
  RemoteRnetLED_ParseCommand,
#endif
#if PL_CONFIG_USE_ROBOT_TO_ESP
  RobotToEsp_ParseCommand,
#endif
  NULL /* Sentinel */
};

#if PL_CONFIG_USE_ESP2ROBOT
  static SemaphoreHandle_t Shell_stdioMutex = NULL; /* mutex used to allow the esp2robot task getting exclusive access to the parsing */

  SemaphoreHandle_t Shell_GetMutex(void) {
    return Shell_stdioMutex;
  }
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

static void ShellTask(void *pvParameters) {
  (void)pvParameters; /* not used */
  McuLog_trace("started shell task");
  /* initialize buffers */
  for(int i=0;i<sizeof(ios)/sizeof(ios[0]);i++) {
    ios[i].buf[0] = '\0';
  }
  McuShell_PrintPrompt(McuShell_GetStdio()); /* print prompt */
  for(;;) {
    /* process all I/Os */
  #if PL_CONFIG_USE_ESP2ROBOT
    if (xSemaphoreTakeRecursive(Shell_stdioMutex, portMAX_DELAY)==pdPASS) { /* take mutex */
  #endif
      for(int i=0;i<sizeof(ios)/sizeof(ios[0]);i++) {
        (void)McuShell_ReadAndParseWithCommandTable(ios[i].buf, ios[i].bufSize, ios[i].stdio, CmdParserTable);
      }
  #if PL_CONFIG_USE_ESP2ROBOT
      (void)xSemaphoreGiveRecursive(Shell_stdioMutex); /* give back mutex */
    }
  #endif
  #if PL_CONFIG_USE_NORDIC_RADIO && RNET_CONFIG_REMOTE_STDIO
    RSTDIO_Print(McuShell_GetStdio()); /* dispatch incoming messages */
  #endif
  #if PL_CONFIG_USE_WATCHDOG
    McuWatchdog_DelayAndReport(McuWatchdog_REPORT_ID_TASK_SHELL, 2, 5);
  #else
    vTaskDelay(pdMS_TO_TICKS(2*5));
  #endif
  } /* for */
}

void Shell_SendChar(unsigned char ch) {
    McuShell_SendCh(ch, ios[0].stdio->stdOut);
}

uint8_t Shell_ParseCommandIO(const unsigned char *command, McuShell_ConstStdIOType *io, bool silent) {
  if (io==NULL) { /* use a default */
#if PL_CONFIG_USE_SHELL_UART
    io = &McuShellUart_stdio;
#elif PL_CONFIG_USE_USB_CDC
    io = &cdc_stdio;
#elif PL_CONFIG_USE_RTT
    io = &McuRTT_stdio;
#else
  #error "no shell std IO?"
#endif
  }
  return McuShell_ParseWithCommandTableExt(command, io, CmdParserTable, silent);
}

uint8_t Shell_ParseCommand(unsigned char *cmd) {
  return McuShell_ParseWithCommandTableExt(cmd, McuShell_GetStdio(), CmdParserTable, false);
}

uint8_t Shell_ParseCommandWithIO(unsigned char *cmd, McuShell_ConstStdIOType *io) {
  return McuShell_ParseWithCommandTableExt(cmd, io, CmdParserTable, false);
}

void Shell_SendStringToIO(const unsigned char *str, McuShell_ConstStdIOType *io) {
  if (io->writeData!=NULL) {
    size_t len = McuUtility_strlen((char*)str);
    io->writeData(str, len);
  } else {
    McuShell_SendStr(str, io->stdOut);
  }
}

void Shell_SendString(const unsigned char *str) {
  Shell_SendStringToIO(str, ios[0].stdio);
}

static void ConfigureLogger(void) {
#if McuLog_CONFIG_IS_ENABLED
	#if McuLog_CONFIG_NOF_CONSOLE_LOGGER==2 && PL_CONFIG_USE_RTT && PL_CONFIG_USE_SHELL_UART /* two loggers possible */
    McuLog_set_console(McuRTT_GetStdio(), 0);
    #if McuLog_CONFIG_USE_COLOR
    McuLog_set_channel_color(0, true); /* enable color for channel zero */
    #endif
    McuLog_set_console(&McuShellUart_stdio, 1);
  #elif 0 && McuLib_CONFIG_CPU_IS_ESP32
    McuLog_set_console(&Uart_stdio, 0);
    #if McuLog_CONFIG_USE_COLOR
    McuLog_set_channel_color(0, true); /* enable color for channel zero */
    #endif
  #elif PL_CONFIG_USE_SHELL_UART /* only UART */
    McuLog_set_console(&McuShellUart_stdio, 0);
  #elif PL_CONFIG_USE_RTT /* only RTT */
    McuLog_set_console(McuRTT_GetStdio(), 0);
  #endif
#endif
}

void Shell_Init(void) {
#if PL_CONFIG_USE_ESP2ROBOT
  Shell_stdioMutex = xSemaphoreCreateRecursiveMutex();
  if (Shell_stdioMutex==NULL) { /* creation failed? */
    McuLog_fatal("Failed creating mutex");
    for(;;);
  }
  vQueueAddToRegistry(SHELL_stdioMutex, "ShellStdIoMutex");
#endif
  ConfigureLogger();
  if (xTaskCreate(ShellTask, "Shell", (2*1024)/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) {
    for(;;){} /* error */
  }
  McuShell_SetStdio(ios[0].stdio);
}

void Shell_Deinit(void) {
  McuShell_SetStdio(NULL);
}

#endif /* PL_CONFIG_USE_SHELL */
