/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief Reflectance sensor hardware configuration.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef REFLECTANCE_SENSOR_CONFIG_H_
#define REFLECTANCE_SENSOR_CONFIG_H_

#ifndef REFELECTANCE_CONFIG_DO_SENSOR_TRACING
  #define REFELECTANCE_CONFIG_DO_SENSOR_TRACING   (0 && PL_CONFIG_USE_RTT)
#endif

/* \todo cleanup */
#define REF_MIN_LINE_VAL          0x200   /* minimum value indicating a line */
#define REF_MIN_NOISE_VAL         0x40    /* values below this are not added to the weighted sum */
#define REF_USE_WHITE_LINE        0       /* if set to 1, then the robot is using a white (on black) line, otherwise a black (on white) line */

#if 0
  /* \todo move this to light coolor sensor*/
  #define LINE_SENSOR_CONFIG_PINS_LED_WHITE_GPIO      GPIOC /*!< White LED GPIO peripheral */
  #define LINE_SENSOR_CONFIG_PINS_LED_WHITE_PORT      PORTC /*!< White LED PORT peripheral */
  #define LINE_SENSOR_CONFIG_PINS_LED_WHITE_PIN       12u   /*!< White LED pin number */

  #define LINE_SENSOR_CONFIG_ENABLE_LED_WHITE_CLOCK() /*!< Enables clock for white LED port */ \
      CLOCK_EnableClock(kCLOCK_PortC);
#endif

#if CONFIG_PLATFORM_IS_ZUMO_FN
  #define REFLECTANCE_CONFIG_NOF_SENSORS  (4) /*!< number of IR sensors*/
  #define REFLECTANCE_CONFIG_SENSOR_PINS()  \
    {.gpio=GPIOD, .port=PORTD, .pin=7},     \
    {.gpio=GPIOD, .port=PORTD, .pin=6},     \
    {.gpio=GPIOD, .port=PORTD, .pin=5},     \
    {.gpio=GPIOD, .port=PORTD, .pin=4},
  #define REFLECTANCE_CONFIG_ENABLE_SENSOR_CLOCK() /*!< Enables clock for Sensor ports */ \
      CLOCK_EnableClock(kCLOCK_PortD);

  #define REFLECTANCE_CONFIG_NOF_IR   (5) /*!< number of IR LEDs */
  #define REFLECTANCE_CONFIG_IR_PINS()   \
    {.gpio=GPIOC, .port=PORTC, .pin=17}, \
    {.gpio=GPIOC, .port=PORTC, .pin=16}, \
    {.gpio=GPIOC, .port=PORTC, .pin=15}, \
    {.gpio=GPIOC, .port=PORTC, .pin=14}, \
    {.gpio=GPIOC, .port=PORTC, .pin=13},
  #define REFLECTANCE_CONFIG_IR_IS_LOW_ACTIVE   true
  #define REFLECTANCE_CONFIG_ENABLE_IR_CLOCK() /*!< Enables clock for Sensor ports */ \
    CLOCK_EnableClock(kCLOCK_PortC);

  #define REF_SENSOR_TIMEOUT_US  500    /* after this time, consider no reflection (black). Must be smaller than the timeout period of the RefCnt timer! */

#elif CONFIG_PLATFORM_IS_ZUMO_FX
  #define REFLECTANCE_CONFIG_NOF_SENSORS  (6) /*!< number of IR sensors*/
  #define REFLECTANCE_CONFIG_SENSOR_PINS()  \
    {.gpio=GPIOD, .port=PORTD, .pin=2},     \
    {.gpio=GPIOD, .port=PORTD, .pin=3},     \
    {.gpio=GPIOD, .port=PORTD, .pin=4},     \
    {.gpio=GPIOD, .port=PORTD, .pin=5},     \
    {.gpio=GPIOD, .port=PORTD, .pin=6},     \
    {.gpio=GPIOD, .port=PORTD, .pin=7},
  #define REFLECTANCE_CONFIG_ENABLE_SENSOR_CLOCK() /*!< Enables clock for Sensor ports */ \
    CLOCK_EnableClock(kCLOCK_PortD);

  #define REFLECTANCE_CONFIG_NOF_IR   (1) /*!< number of IR LEDs */
  #define REFLECTANCE_CONFIG_IR_PINS()   \
    {.gpio=GPIOD, .port=PORTD, .pin=1},
  #define REFLECTANCE_CONFIG_IR_IS_LOW_ACTIVE   false
  #define REFLECTANCE_CONFIG_ENABLE_IR_CLOCK() /*!< Enables clock for Sensor ports */ \
    CLOCK_EnableClock(kCLOCK_PortD);

  #define REF_SENSOR_TIMEOUT_US  500    /* after this time, consider no reflection (black). Must be smaller than the timeout period of the RefCnt timer! */

#endif

#ifndef REFLECTANCE_CONFIG_MININI_FILE_NAME
  #define REFLECTANCE_CONFIG_MININI_FILE_NAME       "settings.ini" /*!< MinINI file name */
#endif

#ifndef REFLECTANCE_CONFIG_MININI_SECTION_REF_CALIBRATION
  #define REFLECTANCE_CONFIG_MININI_SECTION_REF_CALIBRATION              "Reflectance" /*!< MinINI section name */
#endif

#ifndef REFLECTANCE_CONFIG_MININI_KEY_CALIBRATION_MIN
  #define REFLECTANCE_CONFIG_MININI_KEY_CALIBRATION_MIN                  "calibMin" /*!< String key: list of calibration values */
#endif

#ifndef REFLECTANCE_CONFIG_MININI_KEY_CALIBRATION_MAX
  #define REFLECTANCE_CONFIG_MININI_KEY_CALIBRATION_MAX                  "calibMax" /*!< String key: list of calibration values */
#endif

#endif /* REFLECTANCE_SENSOR_CONFIG_H_ */
