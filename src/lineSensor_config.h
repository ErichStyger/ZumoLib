#if 0
/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief Front line sensor hardware configuration.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LINE_SENSOR_CONFIG_H_
#define LINE_SENSOR_CONFIG_H_

#if CONFIG_PLATFORM_IS_ZUMO_2025

    #define LINE_SENSOR_CONFIG_PINS_LED_WHITE_GPIO      GPIOC /*!< White LED GPIO peripheral */
    #define LINE_SENSOR_CONFIG_PINS_LED_WHITE_PORT      PORTC /*!< White LED PORT peripheral */
    #define LINE_SENSOR_CONFIG_PINS_LED_WHITE_PIN       12u   /*!< White LED pin number */

    #define LINE_SENSOR_CONFIG_ENABLE_LED_WHITE_CLOCK() /*!< Enables clock for white LED port */ \
        CLOCK_EnableClock(kCLOCK_PortC);

    #define LINE_SENSOR_CONFIG_PINS_IR_PD_R_GPIO      GPIOC /*!< Right IR LED GPIO peripheral (low-active) */
    #define LINE_SENSOR_CONFIG_PINS_IR_PD_R_PORT      PORTC /*!< Right IR LED PORT peripheral */
    #define LINE_SENSOR_CONFIG_PINS_IR_PD_R_PIN       13u   /*!< Right IR LED pin number */

    #define LINE_SENSOR_CONFIG_PINS_IR_PD_CR_GPIO     GPIOC /*!< Center-right IR LED GPIO peripheral (low-active) */
    #define LINE_SENSOR_CONFIG_PINS_IR_PD_CR_PORT     PORTC /*!< Center-right IR LED PORT peripheral */
    #define LINE_SENSOR_CONFIG_PINS_IR_PD_CR_PIN      14u   /*!< Center-right IR LED pin number */

    #define LINE_SENSOR_CONFIG_PINS_IR_PD_C_GPIO      GPIOC /*!< Center IR LED GPIO peripheral (low-active) */
    #define LINE_SENSOR_CONFIG_PINS_IR_PD_C_PORT      PORTC /*!< Center IR LED PORT peripheral */
    #define LINE_SENSOR_CONFIG_PINS_IR_PD_C_PIN       15u   /*!< Center IR LED pin number */

    #define LINE_SENSOR_CONFIG_PINS_IR_PD_CL_GPIO     GPIOC /*!< Center-left IR LED GPIO peripheral (low-active) */
    #define LINE_SENSOR_CONFIG_PINS_IR_PD_CL_PORT     PORTC /*!< Center-left IR LED PORT peripheral */
    #define LINE_SENSOR_CONFIG_PINS_IR_PD_CL_PIN      16u   /*!< Center-left IR LED pin number */

    #define LINE_SENSOR_CONFIG_PINS_IR_PD_L_GPIO      GPIOC /*!< Left IR LED GPIO peripheral (low-active) */
    #define LINE_SENSOR_CONFIG_PINS_IR_PD_L_PORT      PORTC /*!< Left IR LED PORT peripheral */
    #define LINE_SENSOR_CONFIG_PINS_IR_PD_L_PIN       17u   /*!< Left IR LED pin number */

    #define LINE_SENSOR_CONFIG_ENABLE_IR_LED_CLOCK() /*!< Enables clock for IR LED ports */ \
        CLOCK_EnableClock(kCLOCK_PortC);

    #define LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_R_GPIO      GPIOD /*!< Right photo-transistor GPIO peripheral */
    #define LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_R_PORT      PORTD /*!< Right photo-transistor PORT peripheral */
    #define LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_R_PIN       4u    /*!< Right photo-transistor pin number */

    #define LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_CR_GPIO     GPIOD /*!< Center-right photo-transistor GPIO peripheral */
    #define LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_CR_PORT     PORTD /*!< Center-right photo-transistor PORT peripheral */
    #define LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_CR_PIN      5u    /*!< Center-right photo-transistor pin number */

    #define LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_CL_GPIO     GPIOD /*!< Center-left photo-transistor GPIO peripheral */
    #define LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_CL_PORT     PORTD /*!< Center-left photo-transistor PORT peripheral */
    #define LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_CL_PIN      6u    /*!< Center-left photo-transistor pin number */

    #define LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_L_GPIO      GPIOD /*!< Left photo-transistor GPIO peripheral */
    #define LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_L_PORT      PORTD /*!< Left photo-transistor PORT peripheral */
    #define LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_L_PIN       7u    /*!< Left photo-transistor pin number */

    #define LINE_SENSOR_CONFIG_ENABLE_PHOTO_TRANSISTOR_CLOCK() /*!< Enables clock for photo-transistor ports */ \
        CLOCK_EnableClock(kCLOCK_PortD);
#endif

#endif /* LINE_SENSOR_CONFIG_H_ */

#endif
