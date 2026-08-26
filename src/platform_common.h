/**
 * \file
 * \brief Platform configuration and feature-enable switches.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * Central header that enables or disables subsystems for the Zumo robot.
 */

#ifndef _PLATFORM_COMMON_H
#define _PLATFORM_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#define PL_CONFIG_IS_ROBOT                  (CONFIG_PLATFORM_IS_ZUMO_FX || CONFIG_PLATFORM_IS_ZUMO_FN) /*!< 1: we are a robot */
#define PL_CONFIG_IS_ESP32                  (CONFIG_ESP32_IS_FX_HAT || CONFIG_ESP32_IS_FN_HAT || CONFIG_ESP32_IS_REMOTE) /*!< we are an ESP32*/
/*
 * ******************************   Application ******************************
 */
#ifndef PL_CONFIG_USE_LEDS
  #define PL_CONFIG_USE_LEDS                (1) /*!< 1: has LED support*/
#endif
#ifndef PL_CONFIG_USE_BLINKY
  #define PL_CONFIG_USE_BLINKY              (0 && PL_CONFIG_USE_LEDS) /*!< 1: run blinky LED task */
#endif
#ifndef PL_CONFIG_USE_TIME_DATE
  #define PL_CONFIG_USE_TIME_DATE           (1) /*!< 1: time and date available */
#endif
#ifndef PL_CONFIG_USE_APP_TASK
  #define PL_CONFIG_USE_APP_TASK            (1) /*!< 1: run application task */
#endif
#ifndef PL_CONFIG_USE_MCUFLASH
  #define PL_CONFIG_USE_MCUFLASH            (1) /*!< 1: use internal flash for storage */
#endif
#ifndef PL_CONFIG_USE_MININI
  #define PL_CONFIG_USE_MININI              (1 && PL_CONFIG_USE_MCUFLASH) /*!< use MinINI to store settings */
#endif
#ifndef PL_CONFIG_USE_SEMIHOSTING
  #define PL_CONFIG_USE_SEMIHOSTING         (0 && McuSemihost_CONFIG_IS_ENABLED && PL_CONFIG_IS_ROBOT) /*!< 1: enable semihosting debug output */
#endif

/*
 * ******************************   Shell ******************************
 */
#ifndef PL_CONFIG_USE_SHELL
  #define PL_CONFIG_USE_SHELL               (0) /*!< 1: enable command line shell */
#endif
#ifndef PL_CONFIG_USE_RTT
  #define PL_CONFIG_USE_RTT                 (1 && PL_CONFIG_IS_ROBOT) /*!< 1: enable RTT (SEGGER Real-Time Transfer), including for shell */
#endif
#ifndef PL_CONFIG_USE_SHELL_UART
  #define PL_CONFIG_USE_SHELL_UART          (1) /*!< 1: enable UART for the shell */
#endif
#ifndef PL_CONFIG_USE_TINY_USB
  #define PL_CONFIG_USE_TINY_USB            (1 && CONFIG_PLATFORM_IS_ZUMO_FN) /*!< if using tinyusb stack */
#endif
#ifndef PL_CONFIG_USE_TUD_CDC
  #define PL_CONFIG_USE_TUD_CDC             (1 && PL_CONFIG_USE_TINY_USB) /* tinyUSB CDC device with McuShellCdcDevice */
#endif
#ifndef PL_CONFIG_USE_SHELL_CDC
  #define PL_CONFIG_USE_SHELL_CDC           (1 && PL_CONFIG_IS_ROBOT) /* if using tinyusb CDC as shell interface */
#endif
#ifndef PL_CONFIG_USE_SHELL_RTT
  #define PL_CONFIG_USE_SHELL_RTT           (1 && PL_CONFIG_USE_RTT && PL_CONFIG_IS_ROBOT)
#endif

/*
 * ******************************   Buttons ******************************
 */
#ifndef PL_CONFIG_USE_BUTTONS
  #define PL_CONFIG_USE_BUTTONS             (1 && (CONFIG_ESP32_IS_REMOTE || CONFIG_PLATFORM_IS_ZUMO_FN || CONFIG_PLATFORM_IS_ZUMO_FX)) /*!< 1: enable button driver */
#endif
#ifndef PL_CONFIG_USE_BUTTONS_IRQ
  #define PL_CONFIG_USE_BUTTONS_IRQ         (1 && PL_CONFIG_USE_BUTTONS && !(PL_CONFIG_IS_ESP32 && PL_CONFIG_USE_WIFI)) /* if using button interrupts */
#endif
#ifndef PL_CONFIG_USE_DEBOUNCE
  #define PL_CONFIG_USE_DEBOUNCE            (1 && PL_CONFIG_USE_BUTTONS) /*!< 1: enable software button debouncing (requires PL_CONFIG_USE_BUTTONS) */
#endif

/*
 * ******************************   Robot ******************************
 */
#ifndef PL_CONFIG_USE_ESP32
  #define PL_CONFIG_USE_ESP32               (1 && McuESP32_CONFIG_IS_ENABLED && McuESP32_CONFIG_USE_USB_CDC) /*!< 1: using ESP32 HAT on robot */
#endif
#ifndef PL_CONFIG_USE_ROBOT2ESP
  #define PL_CONFIG_USE_ROBOT2ESP           (1 && PL_CONFIG_USE_ESP32) /*!< 1: shell command communication interface between robot and ESP. Requires PL_CONFIG_USE_ESP2ROBOT on the ESP32 */
#endif
#ifndef PL_CONFIG_HAS_BATTERY_ADC
  #define PL_CONFIG_HAS_BATTERY_ADC         (1 && CONFIG_PLATFORM_IS_ZUMO_FX) /*!< 1: if we can measure the battery voltage with an ADC */
#endif
#ifndef PL_CONFIG_USE_MOTORS
  #define PL_CONFIG_USE_MOTORS              (1 && PL_CONFIG_IS_ROBOT) /*!< 1: enable DC motor driver */
#endif
#ifndef PL_CONFIG_USE_QUADRATURE
  #define PL_CONFIG_USE_QUADRATURE          (1 && PL_CONFIG_USE_MOTORS) /*!< 1: enable quadrature encoder counter */
#endif
#ifndef PL_CONFIG_USE_TACHO
  #define PL_CONFIG_USE_TACHO               (1 && PL_CONFIG_USE_QUADRATURE) /*!< 1: enable tachometer / speed estimation (requires PL_CONFIG_USE_QUADRATURE) */
#endif
#ifndef PL_CONFIG_USE_PID
  #define PL_CONFIG_USE_PID                 (1 && PL_CONFIG_USE_TACHO) /*!< 1: enable PID control loop (requires PL_CONFIG_USE_TACHO) */
#endif
#ifndef PL_CONFIG_USE_POS_PID
  #define PL_CONFIG_USE_POS_PID             (1 && PL_CONFIG_USE_PID) /*!< 1: enable position PID controller (requires PL_CONFIG_USE_PID) */
#endif
#ifndef PL_CONFIG_USE_SPEED_PID
  #define PL_CONFIG_USE_SPEED_PID           (1 && PL_CONFIG_USE_PID) /*!< 1: enable speed PID controller (requires PL_CONFIG_USE_PID) */
#endif
#ifndef PL_CONFIG_USE_DRIVE
  #define PL_CONFIG_USE_DRIVE               (1 && PL_CONFIG_USE_TACHO) /*!< 1: enable drive module (requires PL_CONFIG_USE_TACHO) */
#endif
#ifndef PL_CONFIG_USE_TURN
  #define PL_CONFIG_USE_TURN                (1 && PL_CONFIG_USE_POS_PID) /*!< 1: enable turn/rotation module (requires PL_CONFIG_USE_POS_PID) */
#endif
#ifndef PL_CONFIG_USE_IDENTIFY
  #define PL_CONFIG_USE_IDENTIFY            (1 && CONFIG_PLATFORM_IS_ZUMO_FX) /*!< Used on Vx robots for hardware identification */
#endif
#ifndef PL_CONFIG_USE_ADOPT_HW
  #define PL_CONFIG_USE_ADOPT_HW            (1 && CONFIG_PLATFORM_IS_ZUMO_FX) /*!< Used on Vx robots for hardware adjustments */
#endif
#ifndef PL_CONFIG_USE_BUZZER
  #define PL_CONFIG_USE_BUZZER              (1 && PL_CONFIG_IS_ROBOT) /*!< 1: enable buzzer driver */
#endif
#ifndef PL_CONFIG_USE_REFLECTANCE
  #define PL_CONFIG_USE_REFLECTANCE         (1 && PL_CONFIG_IS_ROBOT) /*!< 1: if having line sensor */
#endif
#ifndef PL_CONFIG_LINE_FOLLOWING
  #define PL_CONFIG_LINE_FOLLOWING          (1 && PL_CONFIG_USE_REFLECTANCE && PL_CONFIG_USE_LINE_PID)
#endif
#ifndef PL_CONFIG_USE_LINE_PID
  #define PL_CONFIG_USE_LINE_PID            (1 && PL_CONFIG_USE_PID && PL_CONFIG_LINE_FOLLOWING)
#endif
#ifndef PL_CONFIG_MAZE_SOLVING  /* \ TODO */
  #define PL_CONFIG_MAZE_SOLVING            (0 && PL_CONFIG_LINE_FOLLOWING)
#endif
#ifndef PL_CONFIG_USE_ROBO_NAV
  #define PL_CONFIG_USE_ROBO_NAV            (1 && PL_CONFIG_USE_DRIVE)
#endif

/*
 * ******************************   I2C, OLED, Sensor ******************************
 */
#ifndef PL_CONFIG_USE_I2C
  #define PL_CONFIG_USE_I2C                 (1 && CONFIG_ESP32_IS_REMOTE) /*!< 1: if using I2C bus */
#endif
#ifndef PL_CONFIG_USE_HW_I2C
  #define PL_CONFIG_USE_HW_I2C              (1 && PL_CONFIG_USE_I2C && CONFIG_USE_HW_I2C)
#endif
#ifndef PL_CONFIG_USE_OLED
  #define PL_CONFIG_USE_OLED                (1 && PL_CONFIG_USE_I2C && CONFIG_ESP32_IS_REMOTE)
#endif
#ifndef PL_CONFIG_USE_SENSIRION
  #define PL_CONFIG_USE_SENSIRION           (0 && PL_CONFIG_USE_I2C && CONFIG_ESP32_IS_REMOTE)
#endif
#ifndef PL_CONFIG_USE_SHT31
  #define PL_CONFIG_USE_SHT31               (1 && PL_CONFIG_USE_SENSIRION) /* board is using SHT31 */
#endif
#ifndef PL_CONFIG_USE_SHT40
  #define PL_CONFIG_USE_SHT40               (!PL_CONFIG_USE_SHT31 && PL_CONFIG_USE_SENSIRION) /* board is using SHT31 */
#endif

/*
 * ******************************   nRF24L01+ ******************************
 */
#ifndef PL_CONFIG_USE_SPI
  #define PL_CONFIG_USE_SPI                 (1 && (CONFIG_PLATFORM_IS_ZUMO_FX || CONFIG_ESP32_IS_REMOTE)) /* if using SPI bus, used for nRF */
#endif
#ifndef PL_CONFIG_USE_NORDIC_RADIO
  #define PL_CONFIG_USE_NORDIC_RADIO        (1 && PL_CONFIG_USE_SPI && McuRNET_CONFIG_IS_ENABLED && McuNRF24L01_CONFIG_IS_ENABLED) /* RNET with nRF24L01+ */
#endif
#ifndef PL_CONFIG_USE_REMOTE_NORDIC
  #define PL_CONFIG_USE_REMOTE_NORDIC       (1 && PL_CONFIG_USE_NORDIC_RADIO) /* using nRF24L01+ transceiver to control robot */
#endif
#ifndef PL_CONFIG_USE_REMOTE_RNET_LED
  #define PL_CONFIG_USE_REMOTE_RNET_LED     (1 && PL_CONFIG_USE_NORDIC_RADIO && PL_CONFIG_USE_LEDS) /* ability to control each other LEDs using nRF */
#endif

/*
 * ******************************   ESP32/WiFi specific settings ******************************
 */
#ifndef PL_CONFIG_USE_WIFI
  #define PL_CONFIG_USE_WIFI                (1 && PL_CONFIG_IS_ESP32) /*!< 1: using WiFi functionality */
#endif
#ifndef PL_CONFIG_USE_ESP_IDENTIFY
  #define PL_CONFIG_USE_ESP_IDENTIFY        (0 && PL_CONFIG_USE_WIFI) /*!< used to identify MAC, needed for EEE network */
#endif
#ifndef PL_CONFIG_USE_PING
  #define PL_CONFIG_USE_PING                (0 && PL_CONFIG_USE_WIFI && MCU_PING_CONFIG_ENABLED) /*!< 1: using ping shell command */
#endif 
#ifndef PL_CONFIG_USE_NTP_CLIENT
  #define PL_CONFIG_USE_NTP_CLIENT          (1 && PL_CONFIG_USE_WIFI && MCU_NTP_CLIENT_CONFIG_ENABLED) /*!< 1: getting time from an NTP server */
#endif
#ifndef PL_CONFIG_USE_UDP_SERVER
  #define PL_CONFIG_USE_UDP_SERVER          (1 && PL_CONFIG_USE_WIFI && MCU_UDP_SERVER_CONFIG_ENABLED) /*!< 1: UDP server task */
#endif
#ifndef PL_CONFIG_USE_UDP_SERVER_BACKEND
  #define PL_CONFIG_USE_UDP_SERVER_BACKEND  (1 && PL_CONFIG_USE_UDP_SERVER) /*!< 1: use backend and callback for UDP messages*/
#endif
#ifndef PL_CONFIG_USE_UDP_CLIENT
  #define PL_CONFIG_USE_UDP_CLIENT          (0 && PL_CONFIG_USE_WIFI && MCU_UDP_CLIENT_CONFIG_ENABLED) /*!< 1: UDP client implementation */
#endif
#ifndef PL_CONFIG_USE_MQTT_CLIENT
  #define PL_CONFIG_USE_MQTT_CLIENT         (1 && PL_CONFIG_USE_WIFI && MCU_MQTT_CLIENT_CONFIG_ENABLED)   /*!< 1: if running MQTT client */
#endif
#ifndef PL_CONFIG_USE_MQTT_SENSOR
  #define PL_CONFIG_USE_MQTT_SENSOR         (1 && PL_CONFIG_USE_WIFI && PL_CONFIG_USE_MQTT_CLIENT && PL_CONFIG_USE_SENSIRION) /*!< 1: using sensor MQTT application */
#endif

#ifndef PL_CONFIG_USE_ESP2ROBOT
  #define PL_CONFIG_USE_ESP2ROBOT           (1 && (CONFIG_ESP32_IS_FX_HAT || CONFIG_ESP32_IS_FN_HAT)) /*!< 1: use command channel from ESP to robot over UART. Requires PL_CONFIG_USE_ROBOT2ESP on the robot. */
#endif
/*
 * ******************************   special hardware related settings ******************************
 */
#ifndef PL_CONFIG_DISABLE_NMI_EZPORT
  #define PL_CONFIG_DISABLE_NMI_EZPORT      (1 && CONFIG_PLATFORM_IS_ZUMO_FN) /*!< 1: disable NMI & EZPORT in startup code, because NMI/PTA4 is used on new robot board */
#endif
#define PL_CONFIG_USE_PICO_W                (0) /* if we are the PicoW board or not */

/*
 * ******************************   Other/legacy specific settings \TODO ******************************
 */
#define PL_CONFIG_USE_NVMC                  (0) /* if using flash storage without MinINI*/
#define PL_CONFIG_HAS_LCD                   (0 && PL_CONFIG_USE_I2C && !PL_CONFIG_USE_OLED && PL_CONFIG_USE_NORDIC_RADIO)
#define PL_CONFIG_HAS_LCD_MENU              (0 && PL_CONFIG_HAS_LCD) /* experimental */
#define PL_CONFIG_USE_MQTT_GAME             (0 && PL_CONFIG_USE_GAME && PL_CONFIG_USE_MQTT_CLIENT) /* if using the code for the MQTT game */

/*
 * ******************************   RS-485 ******************************
 */
#ifndef PL_CONFIG_USE_RS485
  #define PL_CONFIG_USE_RS485               (0 && CONFIG_ESP32_IS_REMOTE && McuUart485_CONFIG_USE_RS_485)
#endif
#ifndef PL_CONFIG_USE_RS485_SHELL
  #define PL_CONFIG_USE_RS485_SHELL         (1 && PL_CONFIG_USE_RS485)
#endif

/*!
 * \brief Initializes the platform and all enabled subsystems.
 */
void Platform_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* _PLATFORM_COMMON_H */