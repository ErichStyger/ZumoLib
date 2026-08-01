/**
 * \file
 * \brief This is the interface to the PID module.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PID_H_
#define PID_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"
#if PL_CONFIG_USE_PID
#include "pid_config.h"
#include <stdint.h>

typedef enum {
  PID_CONFIG_LINE_FW,     /*!< Line follower PID, forward direction */
  PID_CONFIG_LINE_BW,     /*!< Line follower PID, backward direction */
  PID_CONFIG_POS_LEFT,    /*!< Position PID for the left wheel */
  PID_CONFIG_POS_RIGHT,   /*!< Position PID for the right wheel */
  PID_CONFIG_SPEED_LEFT,  /*!< Speed PID for the left wheel */
  PID_CONFIG_SPEED_RIGHT  /*!< Speed PID for the right wheel */
} PID_ConfigType;

typedef struct {
  int32_t pFactor100;       /*!< Proportional factor, scaled by 100 */
  int32_t iFactor100;       /*!< Integral factor, scaled by 100 */
  int32_t dFactor100;       /*!< Derivative factor, scaled by 100 */
  int32_t iAntiWindup;      /*!< Anti-windup limit for the integral term */
  uint8_t maxSpeedPercent;  /*!< Maximum speed percentage (0–100) */
  int32_t lastError;        /*!< Error value from the previous iteration */
  int32_t integral;         /*!< Accumulated integral term */
  uint32_t deadBand;        /*!< Dead-band threshold for position PID */
} PID_Config;

/*!
 * \brief Returns a pointer to the PID configuration for the given controller type.
 * \param config Which PID configuration to retrieve.
 * \param confP  Output pointer that will point to the requested PID_Config.
 * \return Error code, ERR_OK if everything was fine.
 */
uint8_t PID_GetPIDConfig(PID_ConfigType config, PID_Config **confP);

#if PL_CONFIG_USE_SHELL
#include "McuShell.h"
/*!
 * \brief Shell command line parser.
 * \param[in] cmd Pointer to command string
 * \param[out] handled If command is handled by the parser
 * \param[in] io Std I/O handler of shell
 */
uint8_t PID_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif

/*!
 * \brief Performs PID on a line
 * \param currLinePos Current line position
 * \param setLinePos Desired line position
 * \param currLineWidth Indication of line width (in 1000er units for a line)
 * \param forward If we are moving forward or backward
 */
void PID_Line(uint16_t currLinePos, uint16_t setLinePos, uint16_t currLineWidth, bool forward);

/*!
 * \brief Performs PID closed loop calculation for the speed
 * \param currSpeed Current speed of motor
 * \param setSpeed desired speed of motor
 * \param isLeft TRUE if is for the left motor, otherwise for the right motor
 */
void PID_Speed(int32_t currSpeed, int32_t setSpeed, bool isLeft);

/*!
 * \brief Performs PID closed loop calculation for the line position
 * \param currPos Current position of wheel
 * \param setPos Desired wheel position
 * \param isLeft TRUE if is for the left wheel, otherwise for the right wheel
 */
void PID_Pos(int32_t currPos, int32_t setPos, bool isLeft);

/*! \brief Driver initialization */
void PID_Start(void);

/*! \brief Driver initialization */
void PID_Init(void);

/*! \brief Driver de-initialization */
void PID_Deinit(void);

#endif /* PL_CONFIG_USE_PID */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* PID_H_ */
