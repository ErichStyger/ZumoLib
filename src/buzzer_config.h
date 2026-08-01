/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief Buzzer hardware configuration.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BUZZER_CONFIG_H_
#define BUZZER_CONFIG_H_

#if CONFIG_PLATFORM_IS_ZUMO_2025
  #define BUZZER_PINS_USER_GPIO      GPIOC /*!< GPIO peripheral for buzzer pin */
  #define BUZZER_PINS_USER_PORT      PORTC /*!< PORT peripheral for buzzer pin */
  #define BUZZER_PINS_USER_PIN       1u    /*!< Pin number for buzzer */

  #define BUZZER_CONFIG_ENABLE_CLOCK()    /*!< Enables port clock used by buzzer pin */ \
    CLOCK_EnableClock(kCLOCK_PortC);
#elif CONFIG_PLATFORM_IS_ZUMO_Vx
  #define BUZZER_PINS_USER_GPIO      GPIOC /*!< GPIO peripheral for buzzer pin (PTC3, low-active) */
  #define BUZZER_PINS_USER_PORT      PORTC /*!< PORT peripheral for buzzer pin */
  #define BUZZER_PINS_USER_PIN       3u    /*!< Pin number for buzzer */

  #define BUZZER_CONFIG_ENABLE_CLOCK()    /*!< Enables port clock used by buzzer pin */ \
    CLOCK_EnableClock(kCLOCK_PortC);
#endif

#endif /* BUZZER_CONFIG_H_ */
