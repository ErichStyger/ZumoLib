/*
 * Copyright 2026, Erich Styger
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file
 * \brief Main application interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APPLICATION_H
#define _APPLICATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"
#include <stdbool.h>

#if PL_CONFIG_USE_REFLECTANCE
/*!
 * \brief Determines if we are in calibrating state.
 * \return TRUE if calibrating, FALSE otherwise
 */
bool APP_StateIsCalibrating(void);

/*!
 * \brief Starts the calibration sequence
 */
void APP_StateStartCalibrate(void);

/*!
 * \brief Stops the calibration sequence
 */
void APP_StateStopCalibrate(void);

#endif /* PL_CONFIG_USE_REFLECTANCE */

/*!
 * \brief Runs the main application loop. Does not return.
 */
void Application_Run(void);

#if PL_CONFIG_USE_DEBOUNCE
 #include "McuDebounce.h"
 #include "buttons.h"
 #include <stdint.h>
 /*!
  * \brief Callback invoked by the debounce module on button events.
  * \param button The button that triggered the event.
  * \param event The debounce event kind (e.g. press, release, long press).
  */
 void Application_OnButtonEvent(Buttons_e button, McuDbnc_EventKinds event);
#endif

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* _APPLICATION_H */

