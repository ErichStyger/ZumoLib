/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief LED hardware configuration.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LEDS_CONFIG_H_
#define LEDS_CONFIG_H_

#if CONFIG_PLATFORM_IS_ZUMO_FN
  #define LEDS_CONFIG_HAS_LEFT_RED_LED  (1)
  #define LEDS_CONFIG_LEFT_RED_GPIO      GPIOB /*!< GPIO peripheral for left led pin */
  #define LEDS_CONFIG_LEFT_RED_PORT      PORTB /*!< PORT peripheral for left led pin */
  #define LEDS_CONFIG_LEFT_RED_PIN       9u    /*!< Pin number for left led */
  #define LEDS_CONFIG_LEFT_RED_LOW_ACTIVE  (1)

  #define LEDS_CONFIG_HAS_RIGHT_RED_LED (1)
  #define LEDS_CONFIG_RIGHT_RED_GPIO     GPIOE /*!< GPIO peripheral for right led pin */
  #define LEDS_CONFIG_RIGHT_RED_PORT     PORTE /*!< PORT peripheral for right led pin */
  #define LEDS_CONFIG_RIGHT_RED_PIN      2u    /*!< Pin number for right led */
  #define LEDS_CONFIG_RIGHT_RED_LOW_ACTIVE  (1)

  #define LEDS_CONFIG_ENABLE_CLOCK()    /*!< Enables port clock used by left and right led pin */ \
    CLOCK_EnableClock(kCLOCK_PortB); \
    CLOCK_EnableClock(kCLOCK_PortE);
#elif CONFIG_PLATFORM_IS_ZUMO_FX
  #define LEDS_CONFIG_HAS_LEFT_RED_LED  (1)
  #define LEDS_CONFIG_LEFT_RED_GPIO      GPIOA /*!< GPIO peripheral for left led pin */
  #define LEDS_CONFIG_LEFT_RED_PORT      PORTA /*!< PORT peripheral for left led pin */
  #define LEDS_CONFIG_LEFT_RED_PIN       13u    /*!< Pin number for left led */
  #define LEDS_CONFIG_LEFT_RED_LOW_ACTIVE  (1)

  #define LEDS_CONFIG_HAS_RIGHT_RED_LED (1)
  #define LEDS_CONFIG_RIGHT_RED_GPIO     GPIOD /*!< GPIO peripheral for right led pin */
  #define LEDS_CONFIG_RIGHT_RED_PORT     PORTD /*!< PORT peripheral for right led pin */
  #define LEDS_CONFIG_RIGHT_RED_PIN      0u   /*!< Pin number for right led */
  #define LEDS_CONFIG_RIGHT_RED_LOW_ACTIVE  (1)

  #define LEDS_CONFIG_ENABLE_CLOCK()    /*!< Enables port clock used by left and right led pin */ \
    CLOCK_EnableClock(kCLOCK_PortD); \
    CLOCK_EnableClock(kCLOCK_PortA);
#elif McuLib_CONFIG_CPU_IS_ESP32 && CONFIG_ESP32_IS_REMOTE
  #define LEDS_CONFIG_HAS_RED_LED         (1)
  /* red led on IO27, HIGH active */
  #define LEDS_CONFIG_RED_PIN           (GPIO_NUM_27)
  #define LEDS_CONFIG_RED_LOW_ACTIVE    (0)

  #define LEDS_CONFIG_HAS_GREEN_LED       (1)
  /* green led on IO26, HIGH active */
  #define LEDS_CONFIG_GREEN_PIN         (GPIO_NUM_26)
  #define LEDS_CONFIG_GREEN_LOW_ACTIVE  (0)

  #define LEDS_CONFIG_HAS_BLUE_LED        (1)
  /* blue led on IO16, HIGH active */
  #define LEDS_CONFIG_BLUE_PIN          (GPIO_NUM_16)
  #define LEDS_CONFIG_BLUE_LOW_ACTIVE   (0)
#elif McuLib_CONFIG_CPU_IS_ESP32 && CONFIG_ESP32_IS_FN_HAT
  /* No LEDs yet */
#elif McuLib_CONFIG_CPU_IS_ESP32 && CONFIG_ESP32_IS_FX_HAT
  #define LEDS_CONFIG_HAS_RED_LED         (1)
  /* red led on IO27, HIGH active */
  #define LEDS_CONFIG_RED_PIN           (GPIO_NUM_10)
  #define LEDS_CONFIG_RED_LOW_ACTIVE    (1)
#endif

/* disable all LEDs by default */
#ifndef LEDS_CONFIG_ENABLE_CLOCK
  #define LEDS_CONFIG_ENABLE_CLOCK()        /* nothing */
#endif
#ifndef LEDS_CONFIG_HAS_ONBOARD_LED
  #define LEDS_CONFIG_HAS_ONBOARD_LED        (0)
#endif
#ifndef LEDS_CONFIG_HAS_LEFT_RED_LED
  #define LEDS_CONFIG_HAS_LEFT_RED_LED       (0)
#endif
#ifndef LEDS_CONFIG_HAS_RIGHT_RED_LED
  #define LEDS_CONFIG_HAS_RIGHT_RED_LED      (0)
#endif
#ifndef LEDS_CONFIG_HAS_RED_LED
  #define LEDS_CONFIG_HAS_RED_LED            (0)
#endif
#ifndef LEDS_CONFIG_HAS_GREEN_LED
  #define LEDS_CONFIG_HAS_GREEN_LED          (0)
#endif
#ifndef LEDS_CONFIG_HAS_BLUE_LED
  #define LEDS_CONFIG_HAS_BLUE_LED           (0)
#endif
#ifndef LEDS_CONFIG_HAS_ORANGE_LED
  #define LEDS_CONFIG_HAS_ORANGE_LED         (0)
#endif

#endif /* LEDS_CONFIG_H_ */
