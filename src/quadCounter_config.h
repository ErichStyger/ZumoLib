/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief Quadrature counter hardware and feature configuration.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QUADCOUNTER_CONFIG_H_
#define QUADCOUNTER_CONFIG_H_

#ifndef QuadCounter_SWAP_PINS
  #define QuadCounter_SWAP_PINS               0 /*!< 1: C1 and C2 are swapped */
#endif

#ifndef QuadCounter_SWAP_PINS_AT_RUNTIME
  #define QuadCounter_SWAP_PINS_AT_RUNTIME    (1 && CONFIG_PLATFORM_IS_ZUMO_Vx) /*!< 1: C1 and C2 can be swapped at runtime */
#endif

#ifndef QuadCounter_PARSE_COMMAND_ENABLED
  #define QuadCounter_PARSE_COMMAND_ENABLED   1  /*!< 1: ParseCommand() method is present */
#endif

#if CONFIG_PLATFORM_IS_ZUMO_2025
  /*! Left: HS_MOT_L_A PTA13, HS_MOT_L_B PTA12; Right: HS_MOT_R_A PTB19, HS_MOT_R_B PTB18 */
  #define QUADCOUNTER_CONFIG_LEFT_A_GPIO    GPIOA
  #define QUADCOUNTER_CONFIG_LEFT_A_PORT    PORTA
  #define QUADCOUNTER_CONFIG_LEFT_A_PIN     12
  #define QUADCOUNTER_CONFIG_LEFT_B_GPIO    GPIOA
  #define QUADCOUNTER_CONFIG_LEFT_B_PORT    PORTA
  #define QUADCOUNTER_CONFIG_LEFT_B_PIN     13

  #define QUADCOUNTER_CONFIG_RIGHT_A_GPIO   GPIOB
  #define QUADCOUNTER_CONFIG_RIGHT_A_PORT   PORTB
  #define QUADCOUNTER_CONFIG_RIGHT_A_PIN    18
  #define QUADCOUNTER_CONFIG_RIGHT_B_GPIO   GPIOB
  #define QUADCOUNTER_CONFIG_RIGHT_B_PORT   PORTB
  #define QUADCOUNTER_CONFIG_RIGHT_B_PIN    19

  #define QUADCOUNTER_CONFIG_ENABLE_PULL    (1) /*!< 1: enable internal pull resistors */
  #define QUADCOUNTER_CONFIG_ENABLE_CLOCK() /*!< Enables clocks for quadrature ports */ \
    CLOCK_EnableClock(kCLOCK_PortA);        \
    CLOCK_EnableClock(kCLOCK_PortB);

#elif CONFIG_PLATFORM_IS_ZUMO_Vx
  /*! Left: PTC16/PTC17, Right: PTC10/PTC11 */
  #define QUADCOUNTER_CONFIG_LEFT_A_GPIO    GPIOC
  #define QUADCOUNTER_CONFIG_LEFT_A_PORT    PORTC
  #define QUADCOUNTER_CONFIG_LEFT_A_PIN     16
  #define QUADCOUNTER_CONFIG_LEFT_B_GPIO    GPIOC
  #define QUADCOUNTER_CONFIG_LEFT_B_PORT    PORTC
  #define QUADCOUNTER_CONFIG_LEFT_B_PIN     17

  #define QUADCOUNTER_CONFIG_RIGHT_A_GPIO   GPIOC
  #define QUADCOUNTER_CONFIG_RIGHT_A_PORT   PORTC
  #define QUADCOUNTER_CONFIG_RIGHT_A_PIN    10
  #define QUADCOUNTER_CONFIG_RIGHT_B_GPIO   GPIOC
  #define QUADCOUNTER_CONFIG_RIGHT_B_PORT   PORTC
  #define QUADCOUNTER_CONFIG_RIGHT_B_PIN    11

  #define QUADCOUNTER_CONFIG_ENABLE_PULL    (1) /*!< 1: enable internal pull resistors */

  #define QUADCOUNTER_CONFIG_ENABLE_CLOCK() /*!< Enables clocks for quadrature ports */ \
    CLOCK_EnableClock(kCLOCK_PortC);
#endif

#endif /* QUADCOUNTER_CONFIG_H_ */
