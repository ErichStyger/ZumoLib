/*
 * Copyright (c) 2021, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file
 * \brief Reflectance line history sampling interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCES_INTRO_ROBOLIB_LINEHISTORY_H_
#define SOURCES_INTRO_ROBOLIB_LINEHISTORY_H_

#include "reflectance.h"
#if PL_CONFIG_USE_REFLECTANCE
/*!
 * \brief Samples the reflectance sensors and stores history information.
 */
void HISTORY_SampleSensors(void);

/*!
 * \brief Can be called during turning, will use it to sample sensor values.
 */
bool HISTORY_SampleTurnStopFunction(void);

/*!
 * \brief Returns the line kind derived from sampled history.
 * \return Current line kind.
 */
REF_LineKind HISTORY_LineKind(void);

/*!
 * \brief Clears stored line history state.
 */
void HISTORY_Clear(void);
#endif /* PL_CONFIG_USE_REFLECTANCE */

#endif /* SOURCES_INTRO_ROBOLIB_LINEHISTORY_H_ */
