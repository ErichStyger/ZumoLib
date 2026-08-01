/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief Turn-module motion and timeout configuration.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TURN_CONFIG_H_
#define TURN_CONFIG_H_

#define TURN_STEPS_90         680 /*!< Number of encoder steps for a 90-degree turn */
#define TURN_STEPS_LINE       100 /*!< Number of steps to move over the line */
#define TURN_STEPS_PAST_LINE  150 /*!< Number of steps after the line before turning */
     
#define TURN_STEPS_90_TIMEOUT_MS        1000 /*!< Timeout for 90-degree turn in milliseconds */
#define TURN_STEPS_LINE_TIMEOUT_MS      200  /*!< Timeout for step-over-line move in milliseconds */
#define TURN_STEPS_PAST_LINE_TIMEOUT_MS 200  /*!< Timeout for post-line move in milliseconds */
#define TURN_STEPS_STOP_TIMEOUT_MS      150  /*!< Timeout for stop operation in milliseconds */

#define TURN_WAIT_AFTER_STEP_MS         0    /*!< Wait time after a step to let motor PWM become effective */

#endif /* TURN_CONFIG_H_ */
