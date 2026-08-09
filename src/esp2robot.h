/*
 * Copyright (c) 2021, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file
 * \brief ESP-to-robot communication interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ESP_2_ROBOT_H_
#define ESP_2_ROBOT_H_

#include "platform.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if PL_CONFIG_USE_SHELL
  #include "McuShell.h"

  /*!
   * \brief Command line and shell handler
   * \param cmd The command to be parsed
   * \param handled If command has been recognized and handled
   * \param io I/O handler to be used
   * \return error code, otherwise ERR_OK
   */
  uint8_t Esp2robot_ParseCommand(const unsigned char* cmd, bool *handled, const McuShell_StdIOType *io);
#endif /* PL_CONFIG_USE_SHELL */

/*! \brief Module de-initialization */
void Esp2robot_Deinit(void);

/*! \brief Module initialization */
void Esp2robot_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* ESP_2_ROBOT_H_ */
