/*
 * Copyright 2026, Erich Styger
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file
 * \brief Blinky LED task interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BLINKY_H
#define BLINKY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "blinky_config.h"

/*!
 * \brief Initializes the blinky LED task.
 */
void Blinky_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* BLINKY_H */