/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief Motor driver hardware configuration.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOTOR_CONFIG_H_
#define MOTOR_CONFIG_H_

#if CONFIG_PLATFORM_IS_ZUMO_FN
  #define MOTOR_PINS_DIR_LEFT_GPIO      GPIOE /*!< Left motor direction GPIO peripheral */
  #define MOTOR_PINS_DIR_LEFT_PORT      PORTE /*!< Left motor direction PORT peripheral */
  #define MOTOR_PINS_DIR_LEFT_PIN       6u    /*!< Left motor direction pin */

  #define MOTOR_PINS_DIR_RIGHT_GPIO     GPIOE /*!< Right motor direction GPIO peripheral */
  #define MOTOR_PINS_DIR_RIGHT_PORT     PORTE /*!< Right motor direction PORT peripheral */
  #define MOTOR_PINS_DIR_RIGHT_PIN      5u    /*!< Right motor direction pin */

  #define MOTOR_DIR_ENABLE_CLOCK()    /*!< Enables port clocks for direction pins */ \
    CLOCK_EnableClock(kCLOCK_PortE);

  #define MOTOR_INIT_PWM_PINS()  /*!< Initializes PWM output pin muxing */ \
    CLOCK_EnableClock(kCLOCK_PortD); /*!< Port D clock enabled */ \
    PORT_SetPinMux(PORTD, 0U, kPORT_MuxAlt4); /*!< PORTD0 configured as FTM3_CH0 */ \
    PORT_SetPinMux(PORTD, 1U, kPORT_MuxAlt4); /*!< PORTD1 configured as FTM3_CH1 */
  
  #define MOTOR_PWM_FTM_BASEADDR        FTM3                     /*!< FTM peripheral used for motor PWM */
  #define MOTOR_FTM_SOURCE_CLOCK        CLOCK_GetFreq(kCLOCK_BusClk) /*!< Source clock for motor FTM */
  #define MOTOR_PWM_LEFT_FTM_CHANNEL    1U                       /*!< Left motor PWM channel */
  #define MOTOR_PWM_RIGHT_FTM_CHANNEL   0U                       /*!< Right motor PWM channel */

  #define MOTOR_CONFIG_HAS_POWER_ON     (1)     /*!< 1: dedicated pin controls motor power */
  #define MOTOR_PINS_POWER_ON_GPIO      GPIOE   /*!< GPIO peripheral for power-on control */
  #define MOTOR_PINS_POWER_ON_PORT      PORTE   /*!< PORT peripheral for power-on control */
  #define MOTOR_PINS_POWER_ON_PIN       24u     /*!< Pin for power-on control */
  #define MOTOR_PINS_POWER_ON_ENABLE_CLOCK()    /*!< Enables clock for power-on control port */ \
    CLOCK_EnableClock(kCLOCK_PortE);

  #define MOTOR_CONFIG_HAS_MODE         (1)    /*!< 1: MODE pin on DRV8835 is used */
  #define MOTOR_PINS_MODE_GPIO          GPIOC  /*!< GPIO peripheral for MODE pin */
  #define MOTOR_PINS_MODE_PORT          PORTC  /*!< PORT peripheral for MODE pin */
  #define MOTOR_PINS_MODE_PIN           18u    /*!< MODE pin number */
  #define MOTOR_PINS_MODE_ENABLE_CLOCK()    /*!< Enables clock for MODE pin port */ \
    CLOCK_EnableClock(kCLOCK_PortC);

#elif CONFIG_PLATFORM_IS_ZUMO_FX

  #define MOTOR_PINS_DIR_LEFT_GPIO      GPIOC /*!< Left motor direction GPIO peripheral */
  #define MOTOR_PINS_DIR_LEFT_PORT      PORTC /*!< Left motor direction PORT peripheral */
  #define MOTOR_PINS_DIR_LEFT_PIN       9u    /*!< Left motor direction pin */

  #define MOTOR_PINS_DIR_RIGHT_GPIO     GPIOC /*!< Right motor direction GPIO peripheral */
  #define MOTOR_PINS_DIR_RIGHT_PORT     PORTC /*!< Right motor direction PORT peripheral */
  #define MOTOR_PINS_DIR_RIGHT_PIN      8u    /*!< Right motor direction pin */

  #define MOTOR_DIR_ENABLE_CLOCK()    /*!< Enables port clocks for direction pins */ \
    CLOCK_EnableClock(kCLOCK_PortC);

  #define MOTOR_INIT_PWM_PINS()  /*!< Initializes PWM output pin muxing */ \
    CLOCK_EnableClock(kCLOCK_PortC); \
    PORT_SetPinMux(PORTC, 4U, kPORT_MuxAlt4); /*!< PORTC4 configured as FTM0_CH3 */ \
    PORT_SetPinMux(PORTC, 5U, kPORT_MuxAlt7); /*!< PORTC5 configured as FTM0_CH2 */

  #define MOTOR_PWM_FTM_BASEADDR        FTM0                     /*!< FTM peripheral used for motor PWM */
  #define MOTOR_FTM_SOURCE_CLOCK        CLOCK_GetFreq(kCLOCK_BusClk) /*!< Source clock for motor FTM */
  #define MOTOR_PWM_LEFT_FTM_CHANNEL    2U                       /*!< Left motor PWM channel (PTC5/FTM0_CH2) */
  #define MOTOR_PWM_RIGHT_FTM_CHANNEL   3U                       /*!< Right motor PWM channel (PTC4/FTM0_CH3) */

  #define MOTOR_CONFIG_HAS_POWER_ON     (0) /*!< 1: dedicated pin controls motor power */

#endif

#endif /* MOTOR_CONFIG_H_ */
