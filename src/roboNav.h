/**
 * \file
 * \brief Robot navigation with a a nav/joystick (real or virtual).
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

 #ifndef __ROBO_NAV_H__
 #define __ROBO_NAV_H__

 #if PL_CONFIG_USE_SHELL
  #include "McuShell.h"

/*!
 * \brief Shell parser routine.
 * \param cmd Pointer to command line string.
 * \param handled Pointer to status if command has been handled. Set to TRUE if command was understood.
 * \param io Pointer to stdio handle
 * \return Error code, ERR_OK if everything was ok.
 */
  uint8_t RoboNav_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif

#include "buttons.h"
#include "McuDebounce.h"
void RoboNav_OnButtonEvent(Buttons_e button, McuDbnc_EventKinds event);

/*!
 * \brief Module initialization.
 */
void RoboNav_Init(void);

 #endif /* __ROBO_NAV_H__ */