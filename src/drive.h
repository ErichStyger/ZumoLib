/**
 * \file
 * \brief Interface to drive the robot.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * This module allows to drive the robot and to perform turns.
 */

#ifndef DRIVE_H_
#define DRIVE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"
#if PL_CONFIG_USE_DRIVE

#if PL_CONFIG_USE_SHELL
  #include "McuShell.h"

  /*!
  * \brief Parses a command
  * \param cmd Command string to be parsed
  * \param handled Sets this variable to TRUE if command was handled
  * \param io I/O stream to be used for input/output
  * \return Error code, ERR_OK if everything was fine
  */
  uint8_t DRV_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif /* PL_CONFIG_USE_SHELL */

typedef enum {
  DRV_MODE_NONE,    /*!< No drive mode selected */
  DRV_MODE_STOP,    /*!< Motors are stopped */
  DRV_MODE_SPEED,   /*!< Speed-controlled driving mode */
#if PL_CONFIG_USE_QUADRATURE
  DRV_MODE_POS,     /*!< Position-controlled driving mode */
#endif
} DRV_Mode;

/*!
 * \brief Sets the speed for the left and right motors.
 * \param left  Desired speed for the left motor.
 * \param right Desired speed for the right motor.
 * \return Error code, ERR_OK if everything was fine.
 */
uint8_t DRV_SetSpeed(int32_t left, int32_t right);
#if PL_CONFIG_USE_QUADRATURE
  /*!
   * \brief Sets the target encoder position for the left and right wheels.
   * \param left  Target position for the left wheel.
   * \param right Target position for the right wheel.
   * \return Error code, ERR_OK if everything was fine.
   */
  uint8_t DRV_SetPos(int32_t left, int32_t right);
#endif
/*!
 * \brief Returns whether the robot is currently driving backward.
 * \return true if driving backward, false otherwise.
 */
bool DRV_IsDrivingBackward(void);
/*!
 * \brief Sets the drive mode.
 * \param mode New drive mode to apply.
 * \return Error code, ERR_OK if everything was fine.
 */
uint8_t DRV_SetMode(DRV_Mode mode);
/*!
 * \brief Returns the current drive mode.
 * \return Current DRV_Mode value.
 */
DRV_Mode DRV_GetMode(void);
/*!
 * \brief Returns whether the robot has stopped.
 * \return true if all motors are stopped, false otherwise.
 */
bool DRV_IsStopped(void);
/*!
 * \brief Returns whether a turn has been completed.
 * \return true if the turn is done, false otherwise.
 */
bool DRV_HasTurned(void);

/*!
 * \brief Stops the motors.
 * \param timeoutMs Timeout in milliseconds for the operation.
 * \return ERR_OK if stopped, ERR_BUSY for timeout condition.
 */
uint8_t DRV_Stop(int32_t timeoutMs);

/*!
 * \brief Driver initialization.
 */
void DRV_Init(void);

/*!
 * \brief Driver de-initialization.
 */
void DRV_Deinit(void);

#endif /* PL_CONFIG_USE_DRIVE */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* DRIVE_H_ */
