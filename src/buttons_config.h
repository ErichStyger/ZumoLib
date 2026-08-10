/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief Button hardware configuration.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BUTTONS_CONFIG_H_
#define BUTTONS_CONFIG_H_

#if CONFIG_PLATFORM_IS_ZUMO_FN
  #define BUTTONS_CONFIG_PINS_USER_GPIO      GPIOE /*!< GPIO peripheral for user button */
  #define BUTTONS_CONFIG_PINS_USER_PORT      PORTE /*!< PORT peripheral for user button */
  #define BUTTONS_CONFIG_PINS_USER_PIN       26u   /*!< Pin number for user button */

  #define BUTTONS_CONFIG_ENABLE_CLOCK()    /*!< Enables port clock used by button pins */ \
    CLOCK_EnableClock(kCLOCK_PortE);
  
  #define BUTTONS_CONFIG_INTERRUPT_LINE      PORTE_IRQn
  #define BUTTONS_CONFIG_INTERRUPT_HANDLER   PORTE_IRQHandler

#elif CONFIG_PLATFORM_IS_ZUMO_FX
  #define BUTTONS_CONFIG_PINS_USER_GPIO      GPIOA /*!< GPIO peripheral for user button */
  #define BUTTONS_CONFIG_PINS_USER_PORT      PORTA /*!< PORT peripheral for user button */
  #define BUTTONS_CONFIG_PINS_USER_PIN       14u   /*!< Pin number for user button */

  #define BUTTONS_CONFIG_ENABLE_CLOCK()    /*!< Enables port clock used by button pins */ \
    CLOCK_EnableClock(kCLOCK_PortA);

  #define BUTTONS_CONFIG_INTERRUPT_LINE      PORTA_IRQn
  #define BUTTONS_CONFIG_INTERRUPT_HANDLER   PORTA_IRQHandler
#elif McuLib_CONFIG_CPU_IS_ESP32
  /* ESP32: Only pins that support both input & output have integrated pull-up and pull-down resistors. Input-only GPIOs 34-39 do not. */
  #define BUTTONS_PINS_NAVUP_PIN         GPIO_NUM_25
  #define BUTTONS_PINS_NAVDOWN_PIN       GPIO_NUM_39  /* hardware bug: with WiFi enabled, it triggers an interrupt if using interrupts! */
  #define BUTTONS_PINS_NAVLEFT_PIN       GPIO_NUM_35
  #define BUTTONS_PINS_NAVRIGHT_PIN      GPIO_NUM_36  /* hardware bug: with WiFi enabled, it triggers an interrupt if using interrupts! */
  #define BUTTONS_PINS_NAVCENTER_PIN     GPIO_NUM_34
  #define BUTTONS_ENABLE_CLOCK()         /* nothing */

  /* Note: on ESP32, there is a hardware bug, triggering interrupts on GPIO36 (right) and GPIO39 (down).
   * See https://github.com/espressif/esp-idf/commit/d890a516a1097f0a07788e203fdb1a82bb83520e
   * and 3.11 in https://www.espressif.com/sites/default/files/documentation/esp32_errata_en.pdf  */
  #define BUTTONS_CONFIG_USE_IRQ    (!MCU_WIFI_CONFIG_ENABLED) /* if using WiFi on ESP, button pins will generate spurious interrupts */
#endif

#ifndef BUTTONS_CONFIG_USE_IRQ
  #define BUTTONS_CONFIG_USE_IRQ    (1) /*!< if interrupts shall be used */
#endif

typedef enum Buttons_e {
  BUTTONS_NAV_UP,
  BUTTONS_NAV_DOWN,
  BUTTONS_NAV_LEFT,
  BUTTONS_NAV_RIGHT,
  BUTTONS_NAV_CENTER,
  BUTTONS_USER,        /*!< User button */
  BUTTONS_NOF_BUTTONS  /*!< Sentinel, must be last in list */
} Buttons_e;

/*! Bit mask for BUTTONS_USER */
#define BUTTONS_BIT_NAV_UP        (1<<0)
#define BUTTONS_BIT_NAV_DOWN      (1<<1)
#define BUTTONS_BIT_NAV_LEFT      (1<<2)
#define BUTTONS_BIT_NAV_RIGHT     (1<<3)
#define BUTTONS_BIT_NAV_CENTER    (1<<4)
#define BUTTONS_BIT_USER          (1<<5) /* user button */

#endif /* BUTTONS_CONFIG_H_ */
