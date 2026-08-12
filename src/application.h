/*
 * Copyright 2023-2026, Erich Styger
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
#include <stdint.h>

#if PL_CONFIG_USE_REFLECTANCE
/*!
 * \brief Determines if we are in calibrating state.
 * \return TRUE if calibrating, FALSE otherwise
 */
bool Application_StateIsCalibrating(void);

/*!
 * \brief Starts the calibration sequence
 */
void Application_StateStartCalibrate(void);

/*!
 * \brief Stops the calibration sequence
 */
void Application_StateStopCalibrate(void);

#endif /* PL_CONFIG_USE_REFLECTANCE */

#if PL_CONFIG_USE_DEBOUNCE
 #include "McuDebounce.h"
 #include "buttons.h"

 /*!
  * \brief Callback invoked by the debounce module on button events.
  * \param button The button that triggered the event.
  * \param event The debounce event kind (e.g. press, release, long press).
  */
 void Application_OnButtonEvent(Buttons_e button, McuDbnc_EventKinds event);
#endif

#if MCU_MQTT_CLIENT_CONFIG_ENABLED
  void Application_MqttTaskResume(void);
  void Application_MqttTaskSuspend(void);
#endif

/*!
 * \brief Runs the main application loop. Does not return.
 */
void Application_Run(void);

/*!
 * \brief Initializes the application module.
 */
void Application_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* _APPLICATION_H */

