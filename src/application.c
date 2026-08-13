/*
 * Copyright 2026 Erich Styger
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief Main application state machine implementation.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */
  
#include "platform.h"
#include "application.h"
#include "McuRTOS.h"
#include "McuDebounce.h"
#include "buttons.h"
#include "appEsp.h"
#include "appRobot.h"

#if PL_CONFIG_USE_DEBOUNCE
void Application_OnButtonEvent(Buttons_e button, McuDbnc_EventKinds event) { /* called from debouncing */
#if PL_CONFIG_IS_ESP32
  AppEsp_OnButtonEvent(button, event);
#elif PL_CONFIG_IS_ROBOT
  AppRobot_OnButtonEvent(button, event);
#endif
}
#endif /* PL_CONFIG_USE_DEBOUNCE */

void Application_Init(void) {
}
