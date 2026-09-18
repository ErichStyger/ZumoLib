/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief Remote UDP configuration.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef REMOTE_WIFI_UDP_CONFIG_H_
#define REMOTE_WIFI_UDP_CONFIG_H_

#ifndef REMOTE_WIFI_UDP_CONFIG_HOST_NAME
  #define REMOTE_WIFI_UDP_CONFIG_HOST_NAME              "robot-n00" /*!< Remote host default name */
#endif

#ifndef REMOTE_WIFI_UDP_CONFIG_HOST_PORT
  #define REMOTE_WIFI_UDP_CONFIG_HOST_PORT              1234 /*!< Remote port default number */
#endif

#ifndef REMOTE_WIFI_UDP_CONFIG_MININI_FILE_NAME
  #define REMOTE_WIFI_UDP_CONFIG_MININI_FILE_NAME       "settings.ini" /*!< MinINI file name */
#endif

#ifndef REMOTE_WIFI_UDP_CONFIG_MININI_SECTION_NAME
  #define REMOTE_WIFI_UDP_CONFIG_MININI_SECTION_NAME     "RemoteWifiUdp" /*!< MinINI section name */
#endif

#ifndef REMOTE_WIFI_UDP_CONFIG_MININI_KEY_ENABLED
  #define REMOTE_WIFI_UDP_CONFIG_MININI_KEY_ENABLED      "enabled" /*!< int key: 1: enabled, 0: disabled */
#endif

#ifndef REMOTE_WIFI_UDP_CONFIG_MININI_KEY_HOST
  #define REMOTE_WIFI_UDP_CONFIG_MININI_KEY_HOST         "host" /*!< String key: server/host name */
#endif

#ifndef REMOTE_WIFI_UDP_CONFIG_MININI_KEY_PORT
  #define REMOTE_WIFI_UDP_CONFIG_MININI_KEY_PORT         "port" /*!< int key: port number */
#endif

#endif /* REMOTE_WIFI_UDP_CONFIG_H_ */
