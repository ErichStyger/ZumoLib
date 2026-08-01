/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief PID controller tuning constants.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PID_CONFIG_H_
#define PID_CONFIG_H_

#define PID_CONFIG_SPEED_P      (2000)   /*!< Speed PID proportional factor (x100) */
#define PID_CONFIG_SPEED_I      (80)     /*!< Speed PID integral factor (x100) */
#define PID_CONFIG_SPEED_D      (0)      /*!< Speed PID derivative factor (x100) */
#define PID_CONFIG_SPEED_W      (120000) /*!< Speed PID anti-windup limit */
#define PID_CONFIG_SPEED_V      (100)    /*!< Speed PID maximum output percentage */
#define PID_CONFIG_SPEED_B      (0)      /*!< Speed PID dead-band */

#define PID_CONFIG_POS_P        (1000)   /*!< Position PID proportional factor (x100) */
#define PID_CONFIG_POS_I        (1)      /*!< Position PID integral factor (x100) */
#define PID_CONFIG_POS_D        (50)     /*!< Position PID derivative factor (x100) */
#define PID_CONFIG_POS_W        (200)    /*!< Position PID anti-windup limit */
#define PID_CONFIG_POS_V        (20)     /*!< Position PID maximum output percentage */
#define PID_CONFIG_POS_B        (10)     /*!< Position PID dead-band */

#define PID_CONFIG_LINE_P       (5500)   /*!< Forward line PID proportional factor (x100) */
#define PID_CONFIG_LINE_I       (15)     /*!< Forward line PID integral factor (x100) */
#define PID_CONFIG_LINE_D       (100)    /*!< Forward line PID derivative factor (x100) */
#define PID_CONFIG_LINE_W       (10000)  /*!< Forward line PID anti-windup limit */
#define PID_CONFIG_LINE_V       (100)    /*!< Forward line PID maximum output percentage */
#define PID_CONFIG_LINE_B       (0)      /*!< Forward line PID dead-band */

#define PID_CONFIG_LINE_BACK_P  (1000)   /*!< Backward line PID proportional factor (x100) */
#define PID_CONFIG_LINE_BACK_I  (0)      /*!< Backward line PID integral factor (x100) */
#define PID_CONFIG_LINE_BACK_D  (0)      /*!< Backward line PID derivative factor (x100) */
#define PID_CONFIG_LINE_BACK_W  (100000) /*!< Backward line PID anti-windup limit */
#define PID_CONFIG_LINE_BACK_V  (20)     /*!< Backward line PID maximum output percentage */
#define PID_CONFIG_LINE_BACK_B  (0)      /*!< Backward line PID dead-band */

#endif /* PID_CONFIG_H_ */
