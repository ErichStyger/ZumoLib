/*
 * Copyright (c) 2019-2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file
 * \brief OLED display interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef OLED_H_
#define OLED_H_

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#if PL_CONFIG_USE_OLED

#if PL_CONFIG_USE_BUTTONS
  #include "buttons.h"
  #include "McuDebounce.h"

  /*!
   * \brief Handles button events for OLED-related actions.
   * \param button Button that triggered the event.
   * \param kind Debounce event kind.
   */
  void OLED_OnButtonEvent(Buttons_e button, McuDbnc_EventKinds kind);
#endif

/*!
 * \brief Sends text to the OLED display.
 * \param text Text to display.
 */
void OLED_SendText(const char *text);

/*!
 * \brief Initializes the OLED module.
 */
void OLED_Init(void);

/*!
 * \brief Deinitializes the OLED module.
 */
void OLED_Deinit(void);

#endif /* PL_CONFIG_USE_OLED */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* OLED_H_ */
