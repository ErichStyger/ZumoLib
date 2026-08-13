/**
 * \file
 * \brief Application interface for robot specific parts.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

 #ifndef __APP_ROBOT_H__
 #define __APP_ROBOT_H__

 #if PL_CONFIG_USE_DEBOUNCE
 #include "McuDebounce.h"
 #include "buttons.h"

 /*!
  * \brief Callback invoked by the debounce module on button events.
  * \param button The button that triggered the event.
  * \param event The debounce event kind (e.g. press, release, long press).
  */
 void AppRobot_OnButtonEvent(Buttons_e button, McuDbnc_EventKinds event);
#endif

void AppRobot_Main(void);

 void AppRobot_Init(void);

 #endif /* __APP_ROBOT_H__ */