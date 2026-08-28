/*
 * Copyright (c) 2019-2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief Platform initialization implementation.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "McuGPIO.h"
#include "McuLED.h"
#include "McuLog.h"
#include "McuRTOS.h"
#include "McuWait.h"
#include "McuButton.h"
#if configUSE_TIMERS
  #include "FreeRtosTimer.h"
#endif
#if PL_CONFIG_USE_TIME_DATE
  #include "McuTimeDate.h"
#endif
#if PL_CONFIG_USE_SEMIHOSTING
  #include "McuSemihost.h"
#endif
#if configUSE_PERCEPIO_TRACE_HOOKS
  #include "McuPercepio.h"
#elif configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  #include "McuSystemView.h"
#endif
#if PL_CONFIG_USE_SHELL_UART
  #include "McuShellUart.h"
#endif
#if PL_CONFIG_USE_RTT
  #include "McuRTT.h"
#endif
#if PL_CONFIG_USE_SHELL
  #include "shell.h"
#endif
#if PL_CONFIG_USE_TINY_USB
  #include "usb/usb_task.h"
  #include "usb/usb_msc.h"
  #include "usb/usb_cdc.h"
#endif
#if PL_CONFIG_USE_SHELL_CDC
  #include "McuShellCdcDevice.h"
#endif
#if PL_CONFIG_USE_LEDS
  #include "leds.h"
#endif
#if PL_CONFIG_USE_BLINKY
  #include "blinky.h"
#endif
#if PL_CONFIG_USE_BUTTONS
  #include "buttons.h"
#endif
#if PL_CONFIG_USE_DEBOUNCE
  #include "debounce.h"
#endif
#if PL_CONFIG_USE_MCUFLASH
  #include "McuFlash.h"
#endif
#if PL_CONFIG_USE_MININI
  #include "minIni/McuMinINI.h"
#endif
#if PL_CONFIG_USE_I2C
  #include "McuGenericI2C.h"
  #include "McuGenericSWI2C.h"
  #include "McuI2cLib.h"
#endif
#if PL_CONFIG_USE_NVMC
  #include "nvmc.h"
#endif
#include "application.h"

/* robot specifics */
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
#if PL_CONFIG_USE_ROBO_NAV
  #include "roboNav.h"
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
#if PL_CONFIG_LINE_FOLLOWING
  #include "lineFollow.h"
#endif
#if PL_CONFIG_MAZE_SOLVING
  #include "maze.h"
#endif
#if PL_CONFIG_USE_ESP32
  #include "McuESP32.h"
#endif
#if PL_CONFIG_USE_ROBOT2ESP
  #include "robotToEsp.h"
#endif
#if PL_CONFIG_USE_IDENTIFY
  #include "identify.h"
#endif
#if PL_CONFIG_USE_ADOPT_HW
  #include "adaptToHW.h"
#endif
#if PL_CONFIG_HAS_BATTERY_ADC
  #include "battery.h"
#endif

/* Nordic specific */
#if PL_CONFIG_USE_SPI
  #include "McuSPI.h"
#endif
#if PL_CONFIG_USE_NORDIC_RADIO
  #include "RNet_App.h"
#endif
#if PL_CONFIG_USE_REMOTE_NORDIC
  #include "remoteNordic.h"
#endif
#if PL_CONFIG_USE_REMOTE_RNET_LED
  #include "remoteRnetLED.h"
#endif

/* ESP specifics */
#if PL_CONFIG_USE_WIFI
  #include "McuWiFi.h"
#endif
  #include "McuRTOS.h"
#if McuLib_CONFIG_CPU_IS_ESP32
	#include "McuEsp32Mac.h"
	#include "nvs_flash.h"
#endif
#if PL_CONFIG_USE_ESP_IDENTIFY
  #include "esp32_identify.h"
#endif
#if PL_CONFIG_USE_NTP_CLIENT
  #include "McuNtpClient.h"
#endif
#if PL_CONFIG_USE_UDP_CLIENT
  #include "McuUdpClient.h"
#endif
#if PL_CONFIG_USE_UDP_SERVER
  #include "McuUdpServer.h"
#endif
#if PL_CONFIG_USE_UDP_SERVER_BACKEND
  #include "udpServerBackend.h"
#endif
#if PL_CONFIG_USE_ESP2ROBOT
  #include "esp2robot.h"
#endif
#if PL_CONFIG_USE_OLED
  #include "McuSSD1306.h"
  #include "oled.h"
#endif
#if PL_CONFIG_HAS_LCD
  #include "LCD.h"
#endif
#if PL_CONFIG_HAS_LCD_MENU
  #include "LCDMenu.h"
#endif
#if PL_CONFIG_USE_SENSIRION
  #include "sensirion.h"
#endif
#if PL_CONFIG_USE_MQTT_CLIENT
  #include "McuMqttClient.h"
#endif
#if PL_CONFIG_USE_MQTT_SENSOR
  #include "mqtt_sensor.h"
#endif
#if McuUart485_CONFIG_USE_RS_485
  #include "McuUart485.h"
#endif
#if PL_CONFIG_USE_RS485_SHELL
  #include "rs485.h"
#endif

#if PL_CONFIG_IS_ESP32
  #include "appEsp.h"
#elif PL_CONFIG_IS_ROBOT
  #include "appRobot.h"
#endif

#if PL_CONFIG_USE_ESP32
static void Esp32ProgrammingCallback(bool isProgramming) {
  /* need to reduce interrupt and task latency and system load during ESP programming, otherwise USB CDC is not fast enough */
  if (isProgramming) {
  #if PL_CONFIG_USE_QUADRATURE
    Quadrature_StopQuadratureTimer();
  #endif
  #if PL_CONFIG_USE_TACHO
    Tacho_StopSamplingTimer();
  #endif
  #if PL_CONFIG_USE_REFLECTANCE
    Reflectance_Disable();
  #endif
  } else {
  #if PL_CONFIG_USE_REFLECTANCE
    Reflectance_Enable();
  #endif
  #if PL_CONFIG_USE_TACHO
    Tacho_StartSamplingTimer();
  #endif
  #if PL_CONFIG_USE_QUADRATURE
    Quadrature_StartQuadratureTimer();
  #endif
  }
}
#endif /* PL_CONFIG_USE_ESP32 */

#if McuLib_CONFIG_CPU_IS_ESP32
  uint32_t SystemCoreClock = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ*1000000; /* equivalent to ARM CMSIS core clock frequency */
#endif

#if configUSE_HEAP_SCHEME==5
  #if McuLib_CONFIG_CPU_VARIANT==McuLib_CONFIG_CPU_VARIANT_NXP_K22FN
    /* K22FX512 uses SDK linker file, where upper gets filled first by application, and lower is free */
    #define HEAP_SECTION_LOWER_SIZE     (8*1024)
    #define HEAP_SECTION_UPPER_SIZE     (60*1024)
  #else
    /* K22FX512 uses SDK linker file, where upper gets filled first by application, and lower is free */
    #define HEAP_SECTION_LOWER_SIZE     (64*1024)
    #define HEAP_SECTION_UPPER_SIZE     (8*1024)
  #endif
  #define SECTION_NAME_LOWER  ".noinit.LOWER_Heap5"
  #define SECTION_NAME_UPPER ".noinit.UPPER_Heap5"
  static __attribute__ ((used,section(SECTION_NAME_LOWER))) uint8_t heap_sram_lower[HEAP_SECTION_LOWER_SIZE]; /* placed in in no_init section */
  static __attribute__ ((used,section(SECTION_NAME_UPPER))) uint8_t heap_sram_upper[HEAP_SECTION_UPPER_SIZE]; /* placed in in no_init section  */
  static const HeapRegion_t xHeapRegions[] = /* must be from low addresses to high addresses */
  { { &heap_sram_lower[0], sizeof(heap_sram_lower)},
    { &heap_sram_upper[0], sizeof(heap_sram_upper)},
    { NULL, 0 } /* << Terminates the array. */
  };
#endif

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
    #if McuLog_CONFIG_USE_COLOR && McuLib_CONFIG_CPU_IS_ESP32
    McuLog_set_channel_color(0, true); /* enable color for channel zero */
    #endif
  #elif PL_CONFIG_USE_RTT /* only RTT */
    McuLog_set_console(McuRTT_GetStdio(), 0);
  #endif
#endif
}

void Platform_Init(void) {
  McuLib_Init();
  McuWait_Init();
  McuRTOS_Init();
#if configUSE_HEAP_SCHEME==5
  vPortDefineHeapRegions(xHeapRegions); /* Pass the array into vPortDefineHeapRegions(). Must be called first! */
#endif
#if PL_CONFIG_USE_TIME_DATE
  McuTimeDate_Init();
#endif
#if configUSE_TIMERS
  FreeRtosTimer_Init();
#endif
#if PL_CONFIG_USE_RTT
  McuRTT_Init();
#endif
#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  McuSystemView_Init();
#elif configUSE_PERCEPIO_TRACE_HOOKS
  // McuPercepio_Startup(); /* done in McuRTOS_Init() */
  McuPercepio_vTraceEnable(TRC_START);
#endif
  McuGPIO_Init();
  McuLED_Init();
  McuLog_Init();
  ConfigureLogger();
#if PL_CONFIG_USE_MCUFLASH
  McuFlash_Init();
  #if PL_CONFIG_IS_ROBOT
    McuFlash_RegisterMemory((void*)McuMinINI_CONFIG_FLASH_NVM_ADDR_START, McuMinINI_CONFIG_FLASH_NVM_NOF_BLOCKS*McuMinINI_CONFIG_FLASH_NVM_BLOCK_SIZE);
  #elif PL_CONFIG_IS_ESP32
    McuFlash_RegisterMemory((const void*)McuFlash_GetEsp32PartitionAddress(), McuFlash_GetEsp32PartitionSize());
  #endif
#endif
#if PL_CONFIG_USE_MININI
  McuMinINI_Init();
#endif
#if PL_CONFIG_USE_I2C
  McuGenericI2C_Init();
  McuI2cLib_Init();
#endif
#if PL_CONFIG_USE_SHELL_UART
  McuShellUart_Init();
#endif
#if PL_CONFIG_USE_SEMIHOSTING
  McuSemiHost_Init();
#endif
#if PL_CONFIG_USE_LEDS
  Leds_Init();
#endif
#if PL_CONFIG_USE_ESP32
  McuESP32_Init();
  #if McuESP32_CONFIG_IS_ENABLED && McuESP32_CONFIG_USE_USB_CDC
    McuShellCdcDevice_SetRtsCtsCallback(McuESP32_UartStateCallback); /* callback to called in case of RTS/CTS (line status change) */
    McuShellCdcDevice_SetChangeBaudCallback(McuESP32_ChangeUartBaudCallback); /* callback to be called in case of baud rate request */
    McuESP32_SetUsbCdcStdio(McuShellCdcDevice_GetStdio());
    McuESP32_SetUsbCdcIsConnectedCallback(McuShellCdcDevice_IsReady); /* which callback to use to check if we have USB connected */
    McuESP32_SetUsbFlushCallback(McuShellCdcDevice_Flush); /* which callback to call to flush the USB outgoing data */
    McuESP32_SetProgrammingCallback(Esp32ProgrammingCallback); /* callback to reduce system load for improved USB CDC performance */
  #endif
#endif
#if PL_CONFIG_USE_IDENTIFY
  ID_Init();
#endif
#if PL_CONFIG_USE_BUTTONS
  McuBtn_Init();
  Buttons_Init();
#endif
#if PL_CONFIG_USE_DEBOUNCE
  Debounce_Init();
#endif
#if PL_CONFIG_USE_BLINKY
  Blinky_Init();
#endif
#if PL_CONFIG_USE_BUZZER
  Buzzer_Init();
#endif
#if PL_CONFIG_HAS_BATTERY_ADC
  BATT_Init();
#endif
#if PL_CONFIG_USE_MOTORS
  Motor_Init();
#endif
#if PL_CONFIG_USE_QUADRATURE
  QuadCounter_Init();
#endif
#if PL_CONFIG_USE_TACHO
  Tacho_Init();
#endif
#if PL_CONFIG_USE_PID
  PID_Init();
#endif
#if PL_CONFIG_USE_DRIVE
  DRV_Init();
#endif
#if PL_CONFIG_USE_TURN
  TURN_Init();
#endif
#if PL_CONFIG_USE_ROBO_NAV
  RoboNav_Init();
#endif
#if PL_CONFIG_USE_REFLECTANCE
  REF_Init();
#endif
#if PL_CONFIG_LINE_FOLLOWING
  LineFollow_Init();
#endif
#if PL_CONFIG_MAZE_SOLVING
  Maze_Init();
#endif
#if PL_CONFIG_USE_NORDIC_RADIO
  McuSPI_Init();
  RNETA_Init();
#endif
#if PL_CONFIG_USE_REMOTE_NORDIC
  RemoteNordic_Init();
#endif
#if PL_CONFIG_USE_ESP2ROBOT
  Esp2robot_Init();
#endif
#if PL_CONFIG_USE_REMOTE_RNET_LED
  RemoteRnetLED_Init();
#endif
#if PL_CONFIG_USE_ROBOT2ESP
  RobotToEsp_Init();
#endif
#if PL_CONFIG_USE_ADOPT_HW
  ADAPT_AdaptToHardware(); /* must be after quadcounter and motor modules */
#endif
#if PL_CONFIG_USE_UDP_SERVER
  McuUdpServer_Init();
#endif
#if PL_CONFIG_USE_UDP_SERVER_BACKEND
  UdpServerBackend_Init();
#endif
#if PL_CONFIG_USE_UDP_CLIENT
  McuUdpClient_Init();
#endif
#if PL_CONFIG_USE_MQTT_CLIENT
  McuMqttClient_Init();
#endif
#if PL_CONFIG_USE_PING
  McuPing_Init();
#endif
#if PL_CONFIG_USE_NTP_CLIENT
  McuNtpClient_Init();
#endif
#if PL_CONFIG_HAS_LCD
  LCD_Init();
#endif
#if PL_CONFIG_USE_OLED
  OLED_Init();
#endif
#if PL_CONFIG_USE_SENSIRION
  Sensirion_Init();
#endif
#if PL_CONFIG_USE_MQTT_SENSOR
  MqttSensor_Init();
#endif
#if PL_CONFIG_USE_RS485
  #if McuUart485_CONFIG_USE_RAW
    McuUart485_Init();
  #else
    RS485_Init();
  #endif
#endif
#if PL_CONFIG_HAS_LCD_MENU
  LCDMenu_Init();
#endif
#if PL_CONFIG_USE_SHELL
  Shell_Init();
#endif
#if PL_CONFIG_USE_SHELL_CDC
  McuShellCdcDevice_Init();
  #if PL_CONFIG_USE_ESP32 && McuESP32_CONFIG_IS_ENABLED && McuESP32_CONFIG_USE_USB_CDC
    McuShellCdcDevice_SetBufferRxCharCallback(McuESP32_SendTxData); /* store incoming USB CDC character into the ESP outgoing queue */
  #else
    McuShellCdcDevice_SetBufferRxCharCallback(McuShellCdcDevice_QueueData);
  #endif
#endif
#if PL_CONFIG_USE_ESP_IDENTIFY
  ESP32Identify_Init();
#endif
#if PL_CONFIG_USE_WIFI && McuLib_CONFIG_CPU_IS_ESP32
  ESP_ERROR_CHECK(nvs_flash_init()); /* need to call this before using any WiFi functions */
  McuEsp32Mac_Init();
  McuWiFi_Init();
#endif
#if PL_CONFIG_IS_ESP32
  AppEsp_Init();
#elif PL_CONFIG_IS_ROBOT
  AppRobot_Init();
#endif
  Application_Init();
}