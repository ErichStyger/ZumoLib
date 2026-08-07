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
#if 0 && PL_HAS_RADIO && PL_CONFIG_USE_LEDS
  #include "roboLED.h" /* \TODO */
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
#if 0 && PL_HAS_RADIO && PL_CONFIG_USE_LEDS
  RoboLED_ParseCommand, /* \TODO */
#endif
  NULL /* Sentinel */
};

#if PL_CONFIG_USE_ESP2ROBOT
  static SemaphoreHandle_t SHELL_stdioMutex;
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
    if (xSemaphoreTakeRecursive(SHELL_stdioMutex, portMAX_DELAY)==pdPASS) { /* take mutex */
  #endif
      for(int i=0;i<sizeof(ios)/sizeof(ios[0]);i++) {
        (void)McuShell_ReadAndParseWithCommandTable(ios[i].buf, ios[i].bufSize, ios[i].stdio, CmdParserTable);
      }
  #if PL_CONFIG_USE_ESP2ROBOT
      (void)xSemaphoreGiveRecursive(SHELL_stdioMutex); /* give back mutex */
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

uint8_t Shell_ParseCommand(unsigned char *cmd) {
  return McuShell_ParseWithCommandTable(cmd, McuShell_GetStdio(), CmdParserTable);
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

#if 0 && McuLib_CONFIG_CPU_IS_ESP32
/* ----------------- buffer handling for shell messages sent to ESP32 */
static unsigned char *esp_io_buf; /* pointer to buffer */
static size_t esp_io_buf_size; /* size of buffer */

static void esp_io_buf_SendChar(unsigned char ch) {
  McuUtility_chcat(esp_io_buf, esp_io_buf_size, ch);
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
  .echoEnabled = false, /* echo enabled for idf.py monitor */
#endif
};

void Shell_SendToESPAndGetResponse(const unsigned char *msg, unsigned char *response, size_t responseSize) {
  esp_io_buf = response;
  esp_io_buf_size = responseSize;
  esp_io_buf[0] = '\0'; /* initialize buffer */
  McuLog_info("Sending to ESP Shell: %s", msg);
  McuShell_ParseWithCommandTableExt(msg, &esp_stdio, CmdParserTable, true); /* send to ESP32 shell */
  if (response[0]=='\0') { /* empty response? add a default */
    McuUtility_strcpy(response, responseSize, (unsigned char*)"OK"); /* default response */
  }
}
#endif 
/* ----------------------------------------------------------------------*/
#if McuESP32_CONFIG_IS_ENABLED && PL_CONFIG_USE_ESP2ROBOT
void Shell_SendToRobotAndGetResponse(const unsigned char *send, unsigned char *response, size_t responseSize) {
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
  if (xSemaphoreTakeRecursive(SHELL_stdioMutex, portMAX_DELAY)==pdPASS) { /* take mutex */
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
    (void)xSemaphoreGiveRecursive(SHELL_stdioMutex); /* give back mutex */
  }
  if (*response=='\0') { /* if response is empty, send back at least an acknowledgment */
    McuUtility_strcpy(response, responseSize, (unsigned char*)"OK"); /* default response */
  }
#else
  McuUtility_strcpy(response, responseSize, (unsigned char*)"OK"); /* default response */
#endif
}
#endif /* PL_CONFIG_USE_ESP2ROBOT */
/* ----------------------------------------------------------------------*/
#if McuESP32_CONFIG_IS_ENABLED
/* write output from the ESP to the robot shell too */
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

McuShell_ConstStdIOTypePtr Shell_GetIOforEspRx(void) {
  return &McuShellUart_stdio; /* send ESP data to K22 UART */
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
  SHELL_stdioMutex = xSemaphoreCreateRecursiveMutex();
  if (SHELL_stdioMutex==NULL) { /* creation failed? */
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
