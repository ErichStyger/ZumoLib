/**
 * \file
 * \brief This is the interface to the FreeRTOS timer module
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TIMER_H_
#define __TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Initializes the FreeRTOS timer module.
 */
void FreeRtosTimer_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __TIMER_H_ */
