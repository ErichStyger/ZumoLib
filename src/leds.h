/*
 * Copyright (c) 2023-2024, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file
 * \brief LED driver handles and lifecycle interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MY_LEDS_H_
#define MY_LEDS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "leds_config.h"
#include "platform.h"

#if PL_CONFIG_USE_SHELL
  #include <stdint.h>
  #include <stdbool.h>
  #include "McuShell.h"

  /*!
   * \brief Command line handler
   * \param cmd String to command to be parsed
   * \param handled Returns if command has been handled
   * \param io I/O channel
   * \return Error code, ERR_OK if everything is ok
   */
  uint8_t Leds_ParseCommand(const uint8_t *cmd, bool *handled, McuShell_ConstStdIOType *io);
#endif

/*!
 * handles for the different LEDs
 */
typedef enum LEDS_Leds_e {
#if LEDS_CONFIG_HAS_LEFT_RED_LED
  LEDS_LEFT_RED,
#endif
#if LEDS_CONFIG_HAS_RIGHT_RED_LED
  LEDS_RIGHT_RED,
#endif
#if LEDS_CONFIG_HAS_RED_LED
  LEDS_RED,
#endif
#if LEDS_CONFIG_HAS_GREEN_LED
  LEDS_GREEN,
#endif
#if LEDS_CONFIG_HAS_BLUE_LED
  LEDS_BLUE,
#endif
#if LEDS_CONFIG_HAS_ONBOARD_LED
  LEDS_ONBOARD,
#endif
  LEDS_NOF_LEDS, /*!< Sentinel, must be last! */
} LEDS_Leds_e;

/*!
 * \brief Turn LED on
 * \param led LED handle
 */
void Leds_On(LEDS_Leds_e led);

/*!
 * \brief Turn LED off
 * \param led LED handle
 */
void Leds_Off(LEDS_Leds_e led);

/*!
 * \brief Toggle LED
 * \param led LED handle
 */
void Leds_Neg(LEDS_Leds_e led);

/*!
 * \brief Return the status (on/off) of the LED
 * \param led LED handle
 * \return true if LED is on, false otherwise
 */
bool Leds_Get(LEDS_Leds_e led);

/*!
 * \brief Module initialization, call it to initialize the driver
 */
void Leds_Init(void);

/*!
 * \brief Initialization to be called from a task context, for example for the on-board LED of the Pico-W
 */
void Leds_InitFromTask(void);

/*!
 * \brief Module de-initialization, call it to de-initialize the driver
 */
void Leds_Deinit(void);

/*!
 * \brief De-initializes the LED module.
 */
void Leds_Deinit(void);
/*!
 * \brief Initializes the LED module.
 */
void Leds_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* MY_LEDS_H_ */
