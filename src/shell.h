/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file
 * \brief Command line shell interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SHELL_H_
#define SHELL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Module de-initialization
 */
void Shell_Deinit(void);

/*!
 * \brief Module Initialization
 */
void Shell_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* SHELL_H_ */
