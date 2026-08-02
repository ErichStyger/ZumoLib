/*
 * Copyright (c) 2019-2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
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

  void OLED_OnButtonEvent(BTN_Buttons_e button, McuDbnc_EventKinds kind);
#endif

void OLED_SendText(const char *text);

void OLED_Init(void);
void OLED_Deinit(void);

#endif /* PL_CONFIG_USE_OLED */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* OLED_H_ */
