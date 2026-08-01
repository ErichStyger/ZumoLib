/*
 * Copyright (c) 2021, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file
 * \brief Maze solving strategy and path management interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef Maze_H_
#define Maze_H_

#include "platform.h"
#if PL_CONFIG_MAZE_SOLVING
#include "turn.h"
#include "reflectance.h"

/*!
 * \brief Adds a new path while going forward through the maze
 * \param kind New path to be added
 */
void Maze_AddPath(TURN_Kind kind);

/*!
 * \brief Tries to simplify the path, basically cutting dead end paths.
 */
void Maze_SimplifyPath(void);

/*!
 * \brief Returns TRUE if the maze has been solved (finish has been found)
 * \return TRUE if finish has been found, so maze is solved
 */
bool Maze_IsSolved(void);

/*!
 * \brief Marks the maze as solved.
 */
void Maze_SetSolved(void);

/*!
 * This clears the solution, and Maze_IsSolved() will return FALSE
 */
void Maze_ClearSolution(void);

/*!
 * \brief Used to get the solution turns, one after each other
 * \param solvedIdx Solution index, starting with zero. The callee will increment the index.
 * \return Solution turn
 */
TURN_Kind Maze_GetSolvedTurn(uint8_t *solvedIdx);

/*!
 * \brief Selects the new turn based.
 * \param prev Line previous the intersection
 * \param curr Line kind of the intersection.
 * \return The new turn.
 */
TURN_Kind Maze_SelectTurn(REF_LineKind prev, REF_LineKind curr);

/*!
 * \brief Function which returns the current strategy
 * \return Returns TRUE if using left-hand-on-the-wall, FALSE otherwise
 */
bool Maze_IsLeftHandRule(void);

/*!
 * \brief Performs a turn while going forward over a line.
 * \param finished TRUE if reached finished area
 * \param deadEndGoBw TRUE if a dead end was found and the robot is going backward
 * \return Returns TRUE while turn is still in progress.
 */
uint8_t Maze_EvaluteTurn(bool *finished, bool *deadEndGoBw);

/*!
 * \brief Evaluates and performs a turn while driving backward.
 * \return Error code, ERR_OK if everything was fine.
 */
uint8_t Maze_EvaluateTurnBw(void);

#if PL_CONFIG_USE_SHELL
#include "McuShell.h"
/*!
 * \brief Module command line parser
 * \param cmd Pointer to command string to be parsed
 * \param handled Set to TRUE if command has handled by parser
 * \param io Shell standard I/O handler
 * \return Error code, ERR_OK if everything was ok
 */
uint8_t Maze_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif

/*!
 * \brief Selects left-hand or right-hand maze solving rule.
 * \param isLeft TRUE to use left-hand rule, FALSE for right-hand rule.
 * \return Error code, ERR_OK if everything was fine.
 */
uint8_t Maze_SetHandRule(bool isLeft);

/*!
 * \brief Module initialization.
 */
void Maze_Init(void);

#endif /* PL_CONFIG_MAZE_SOLVING */

#endif /* Maze_H_ */
