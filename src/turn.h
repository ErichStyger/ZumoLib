/**
 * \file
 * \brief Robot turning interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * This module implements turning of the robot.
 */

#ifndef TURN_H_
#define TURN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"
#if PL_CONFIG_USE_TURN
#include "turn_config.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
  TURN_LEFT45,   /*!< Turn 45 degrees left and stop */
  TURN_LEFT90,   /*!< Turn 90 degrees left and stop */
  TURN_RIGHT45,  /*!< Turn 45 degrees right and stop */
  TURN_RIGHT90,  /*!< Turn 90 degrees right and stop */
  TURN_LEFT180,  /*!< Turn 180 degrees counterclockwise and stop */
  TURN_RIGHT180, /*!< Turn 180 degrees clockwise and stop */
  TURN_STRAIGHT, /*!< Drive straight, no turning */
  TURN_STEP_LINE_FW,                  /*!< Step forward over the line */
  TURN_STEP_LINE_FW_AND_PAST_LINE,    /*!< Step forward over the line and past it for a subsequent turn */
  TURN_STEP_LINE_BW_AND_PAST_LINE,    /*!< Step backward over the line and past it for a subsequent turn */
  TURN_STEP_LINE_BW,                  /*!< Step backward over the line */
  TURN_STEP_BORDER_BW,                /*!< Step back from the Sumo arena border */
  TURN_STEP_PAST_LINE_FW,             /*!< Step past the line forward, before a turn */
  TURN_STEP_PAST_LINE_BW,             /*!< Step past the line backward, before a turn */
  TURN_FINISH,                        /*!< Stepped into the finish zone */
  TURN_STOP_LEFT,   /*!< Stop the left motor only */
  TURN_STOP_RIGHT,  /*!< Stop the right motor only */
  TURN_STOP         /*!< Stop both motors */
} TURN_Kind;

/*! \brief Callback type function to stop process or turning */
typedef bool (*TURN_StopFct)(void);

/*!
 * \brief Translate a turn kind into a string
 * \return String, like "STOP"
 */
const unsigned char *TURN_TurnKindStr(TURN_Kind kind);

/*!
 * \brief Turns the robot.
 * \param kind How much the robot has to turn.
 * \param stopIt Callback to stop turning, or NULL.
 */
void TURN_Turn(TURN_Kind kind, TURN_StopFct stopIt);

/*!
 * \brief Turn robot into position.
 * \param targetLPos Left wheel position.
 * \param targetRPos Right wheel position.
 * \param wait Wait until it is in position.
 * \param stopIt Callback to stop turning, or NULL.
 * \param timeoutMs Timeout value in milliseconds for the turning operation.
 */
void TURN_MoveToPos(int32_t targetLPos, int32_t targetRPos, bool wait, TURN_StopFct stopIt, int32_t timeoutMs);

/*!
 * \brief Turn by angle
 * \param angle Angle, negative angle means left turn, positive means right turn
 * \param stopIt Callback to stop turning, or NULL.
 */
void TURN_TurnAngle(int16_t angle, TURN_StopFct stopIt);

/*!
 * \brief Sets the encoder step counts used for turn calculations.
 * \param steps90      Number of encoder steps for a 90-degree turn.
 * \param stepsLine    Number of encoder steps to reach the line.
 * \param stepsPostLine Number of encoder steps to move past the line.
 */
void TURN_SetSteps(int32_t steps90, int32_t stepsLine, int32_t stepsPostLine);

#if PL_CONFIG_USE_SHELL
#include "McuShell.h"
/*!
 * \brief Shell command line parser.
 * \param[in] cmd Pointer to command string
 * \param[out] handled If command is handled by the parser
 * \param[in] io Std I/O handler of shell
 */
uint8_t TURN_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif

/*!
 * \brief Initializes the module.
 */
void TURN_Init(void);

#endif /* PL_CONFIG_USE_TURN */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* TURN_H_ */
