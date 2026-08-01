/*
 * Copyright (c) 2021-2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file
 * \brief Quadrature encoder counter interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * This module tracks the quadrature encoder signals for the left and right wheels.
 */

#ifndef __QuadCounter_H
#define __QuadCounter_H

#ifdef __cplusplus
extern "C" {
#endif

#include "quadCounter_config.h" /*!< Configuration */
#include "McuShell.h"

typedef uint32_t QuadCounter_QuadCntrType; /*!< Type used for quadrature counter position values */

/*!
 * \brief Sets the position counter for the left wheel.
 * \param pos Position value to set.
 */
void QuadCounter_SetPosLeft(QuadCounter_QuadCntrType pos);

/*!
 * \brief Sets the position counter for the right wheel.
 * \param pos Position value to set.
 */
void QuadCounter_SetPosRight(QuadCounter_QuadCntrType pos);

/*!
 * \brief Swaps (inverts) the A/B encoder pins for the left wheel at runtime.
 * \param swap true to swap pins, false for normal order.
 */
void QuadCounter_SwapPinsLeft(bool swap);

/*!
 * \brief Swaps (inverts) the A/B encoder pins for the right wheel at runtime.
 * \param swap true to swap pins, false for normal order.
 */
void QuadCounter_SwapPinsRight(bool swap);

/*!
 * \brief Returns the accumulated error count for the left encoder.
 * \return Number of decoding errors since initialization.
 */
uint32_t QuadCounter_GetErrorsLeft(void);

/*!
 * \brief Returns the accumulated error count for the right encoder.
 * \return Number of decoding errors since initialization.
 */
uint32_t QuadCounter_GetErrorsRight(void);

/*!
 * \brief Enables pull-up resistors on the encoder input pins.
 */
void QuadCounter_EnablePullups(void);

/*!
 * \brief Returns the current position of the left wheel encoder.
 * \return Current left encoder position.
 */
QuadCounter_QuadCntrType QuadCounter_GetPosLeft(void);

/*!
 * \brief Returns the current position of the right wheel encoder.
 * \return Current right encoder position.
 */
QuadCounter_QuadCntrType QuadCounter_GetPosRight(void);

/*!
 * \brief Periodically samples the encoder signals. Must be called at a fixed rate.
 */
void QuadCounter_Sample(void);

/*!
 * \brief De-initializes the quadrature counter module.
 */
void QuadCounter_Deinit(void);

/*!
 * \brief Parses a shell command for the quadrature counter module.
 * \param cmd     Command string to be parsed.
 * \param handled Set to TRUE if the command was handled.
 * \param io      I/O stream used for input/output.
 * \return Error code, ERR_OK if everything was fine.
 */
uint8_t QuadCounter_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*!
 * \brief Returns the raw quadrature value (0–3) for the left encoder.
 * \return Quadrature value of the left encoder.
 */
uint8_t QuadCounter_GetValLeft(void);

/*!
 * \brief Returns the raw quadrature value (0–3) for the right encoder.
 * \return Quadrature value of the right encoder.
 */
uint8_t QuadCounter_GetValRight(void);

/*!
 * \brief Initializes the quadrature counter module.
 */
void QuadCounter_Init(void);

/*!
 * \brief Starts the hardware timer used for quadrature decoding.
 */
void Quadrature_StartQuadratureTimer(void);

/*!
 * \brief Stops the hardware timer used for quadrature decoding.
 */
void Quadrature_StopQuadratureTimer(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __QuadCounter_H */
