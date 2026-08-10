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

#define PL_CONFIG_DISABLE_NMI_EZPORT    (1 && CONFIG_PLATFORM_IS_ZUMO_2025) /*!< 1: disable NMI & EZPORT in startup code, because NMI/PTA4 is used on new robot board */

#ifndef PL_CONFIG_USE_LEDS
  #define PL_CONFIG_USE_LEDS              (1)
#endif

#ifndef PL_CONFIG_USE_BLINKY
  #define PL_CONFIG_USE_BLINKY            (0 && PL_CONFIG_USE_LEDS) /*!< 1: enable blinky LED task */
#endif
#ifndef PL_CONFIG_USE_APP_TASK
  #define PL_CONFIG_USE_APP_TASK          (1) /*!< 1: enable application task */
#endif
#ifndef PL_CONFIG_USE_SHELL
  #define PL_CONFIG_USE_SHELL             (0) /*!< 1: enable command line shell */
#endif
#ifndef PL_CONFIG_USE_RTT
  #define PL_CONFIG_USE_RTT               (0) /*!< 1: enable RTT (SEGGER Real-Time Transfer), including for shell */
#endif
#ifndef PL_CONFIG_USE_SHELL_UART
  #define PL_CONFIG_USE_SHELL_UART        (0) /*!< 1: enable LPUART as shell transport */
#endif

#ifndef PL_CONFIG_USE_BUTTONS
  #define PL_CONFIG_USE_BUTTONS           (0) /*!< 1: enable button driver */
#endif
#ifndef PL_CONFIG_USE_DEBOUNCE
  #define PL_CONFIG_USE_DEBOUNCE          (1 && PL_CONFIG_USE_BUTTONS) /*!< 1: enable software button debouncing (requires PL_CONFIG_USE_BUTTONS) */
#endif
#ifndef PL_CONFIG_USE_MOTORS
  #define PL_CONFIG_USE_MOTORS            (0) /*!< 1: enable DC motor driver */
#endif
#ifndef PL_CONFIG_USE_QUADRATURE
  #define PL_CONFIG_USE_QUADRATURE        (1 && PL_CONFIG_USE_MOTORS) /*!< 1: enable quadrature encoder counter */
#endif
#ifndef PL_CONFIG_USE_TACHO
  #define PL_CONFIG_USE_TACHO             (1 && PL_CONFIG_USE_QUADRATURE) /*!< 1: enable tachometer / speed estimation (requires PL_CONFIG_USE_QUADRATURE) */
#endif
#ifndef PL_CONFIG_USE_PID
  #define PL_CONFIG_USE_PID               (1 && PL_CONFIG_USE_TACHO) /*!< 1: enable PID control loop (requires PL_CONFIG_USE_TACHO) */
#endif
#ifndef PL_CONFIG_USE_POS_PID
  #define PL_CONFIG_USE_POS_PID           (1 && PL_CONFIG_USE_PID) /*!< 1: enable position PID controller (requires PL_CONFIG_USE_PID) */
#endif
#ifndef PL_CONFIG_USE_SPEED_PID
  #define PL_CONFIG_USE_SPEED_PID         (1 && PL_CONFIG_USE_PID) /*!< 1: enable speed PID controller (requires PL_CONFIG_USE_PID) */
#endif
#ifndef PL_CONFIG_USE_DRIVE
  #define PL_CONFIG_USE_DRIVE             (1 && PL_CONFIG_USE_TACHO) /*!< 1: enable drive module (requires PL_CONFIG_USE_TACHO) */
#endif
#ifndef PL_CONFIG_USE_TURN
  #define PL_CONFIG_USE_TURN              (1 && PL_CONFIG_USE_POS_PID) /*!< 1: enable turn/rotation module (requires PL_CONFIG_USE_POS_PID) */
#endif
#ifndef PL_CONFIG_USE_IDENTIFY
  #define PL_CONFIG_USE_IDENTIFY          (1 && CONFIG_PLATFORM_IS_ZUMO_Vx) /*!< Used on Vx robots for hardware identification */
#endif
#ifndef PL_CONFIG_USE_ADOPT_HW
  #define PL_CONFIG_USE_ADOPT_HW          (1 && CONFIG_PLATFORM_IS_ZUMO_Vx) /*!< Used on Vx robots for hardware adjustments */
#endif

#ifndef PL_CONFIG_USE_BUZZER
  #define PL_CONFIG_USE_BUZZER              (0) /*!< 1: enable buzzer driver */
#endif
#ifndef PL_CONFIG_USE_REFLECTANCE
  #define PL_CONFIG_USE_REFLECTANCE         (0) /*!< 1: if having line sensor */
#endif
#ifndef PL_CONFIG_LINE_FOLLOWING
  #define PL_CONFIG_LINE_FOLLOWING          (0 && PL_CONFIG_USE_REFLECTANCE)
#endif
#ifndef PL_CONFIG_USE_LINE_PID
  #define PL_CONFIG_USE_LINE_PID            (PL_CONFIG_USE_PID && PL_CONFIG_LINE_FOLLOWING)
#endif
#ifndef PL_CONFIG_MAZE_SOLVING
  #define PL_CONFIG_MAZE_SOLVING            (0 && PL_CONFIG_LINE_FOLLOWING)
#endif

#ifndef PL_CONFIG_USE_MCUFLASH
  #define PL_CONFIG_USE_MCUFLASH            (1)
#endif
#ifndef PL_CONFIG_USE_MININI
  #define PL_CONFIG_USE_MININI              (1 && PL_CONFIG_USE_MCUFLASH)
#endif
#ifndef PL_CONFIG_USE_TINY_USB
  #define PL_CONFIG_USE_TINY_USB            (1 && CONFIG_PLATFORM_IS_ZUMO_2025) /*!< if using tinyusb stack */
#endif
#ifndef PL_CONFIG_USE_TUD_CDC
  #define PL_CONFIG_USE_TUD_CDC             (1 && PL_CONFIG_USE_TINY_USB) /* tinyUSB CDC device with McuShellCdcDevice */
#endif
#ifndef PL_CONFIG_USE_SHELL_CDC
  #define PL_CONFIG_USE_SHELL_CDC           (0) /* if using CDC as shell interface */
#endif
#ifndef PL_CONFIG_USE_SHELL_RTT
  #define PL_CONFIG_USE_SHELL_RTT           (1 && PL_CONFIG_USE_RTT)
#endif

#ifndef PL_CONFIG_USE_SEMIHOSTING
  #define PL_CONFIG_USE_SEMIHOSTING         (0 && McuSemihost_CONFIG_IS_ENABLED) /*!< 1: enable semihosting debug output */
#endif

#ifndef PL_CONFIG_USE_I2C
  #define PL_CONFIG_USE_I2C                 (1) /* if using I2C e.g. for accelerometer */
#endif

#ifndef PL_CONFIG_USE_ESP32
  #define PL_CONFIG_USE_ESP32               (0 && McuESP32_CONFIG_IS_ENABLED && McuESP32_CONFIG_USE_USB_CDC)
#endif

#ifndef PL_CONFIG_USE_NORDIC_RADIO
  #define PL_CONFIG_USE_NORDIC_RADIO        (1 && (CONFIG_PLATFORM_IS_ZUMO_Vx || CONFIG_ESP32_IS_REMOTE)) /* using the nRF transceiver */
#endif

#ifndef PL_CONFIG_USE_REMOTE_RNET_LED
  #define PL_CONFIG_USE_REMOTE_RNET_LED     (1 && PL_CONFIG_USE_NORDIC_RADIO && McuRNET_CONFIG_IS_ENABLED && PL_CONFIG_USE_LEDS)
#endif

#ifndef PL_CONFIG_USE_TIME_DATE
  #define PL_CONFIG_USE_TIME_DATE           (1) /* having time and date available */
#endif

/*!
 * \brief Initializes the platform and all enabled subsystems.
 */
void Platform_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* _PLATFORM_COMMON_H */