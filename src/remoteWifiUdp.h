/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file
 * \brief Remote controller interface over WiFi and UDP
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _REMOTE_WIFI_UDP_H__
#define _REMOTE_WIFI_UDP_H__

#ifdef __cplusplus
extern "C" {
#endif

#if McuLib_CONFIG_CPU_IS_ESP32
  #include "buttons.h"
  #include "McuDebounce.h"
  /*!
    * \brief Forwards an ESP32 button event to the Nordic remote handling.
    * \param button Button that triggered the event.
    * \param event Debounce event kind.
    */
  void RemoteWifiUdp_EspOnButtonEvent(Buttons_e button, McuDbnc_EventKinds event);
#endif

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* _REMOTE_WIFI_UDP_H__ */