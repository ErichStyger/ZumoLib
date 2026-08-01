/**
 * \file
 * \brief Motor driver interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * This module implements a driver for up to two small DC motors.
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"
#if PL_CONFIG_USE_MOTORS
#include "motor_config.h"
#include <stdint.h>
#include <stdbool.h>

#define MOTOR_HAS_INVERT 1  /*!< 1: support motor direction inversion at runtime */

typedef enum {
  MOTOR_DIR_FORWARD,  /*!< Motor forward direction */
  MOTOR_DIR_BACKWARD  /*!< Motor backward direction */
} Motor_Direction;

typedef int8_t Motor_SpeedPercent; /*!< -100%...+100%, where negative is backward */

typedef struct Motor_MotorDevice {
#if MOTOR_HAS_INVERT
  bool inverted; /*!< true if direction logic is inverted */
#endif
  Motor_SpeedPercent currSpeedPercent; /*!< our current speed in %, negative percent means backward */
  uint16_t currPWMvalue; /*!< current PWM value used */
  void (*SetRatio16)(uint16_t); /*!< function to set the ratio */
  void (*DirPutVal)(bool); /*!< function to set direction bit */
#if PL_HAS_MOTOR_BRAKE
  bool brake; /*!< true if brake is enabled */
  void (*BrakePutVal)(bool); /*!< function to set brake bit */
#endif
#if PL_HAS_MOTOR_CURRENT_SENSE
  uint16_t currentValue; /*!< current ADC current sensor value */
  uint16_t offset; /*!< current ADC sensor offset value */
#endif
} Motor_MotorDevice;

typedef enum {
  MOTOR_SIDE_LEFT, /*!< left motor */
  MOTOR_SIDE_RIGHT /*!< right motor */
} Motor_MotorSide;

#if MOTOR_HAS_INVERT
/*!
 * \brief Inverts the forward/backward signal for a motor
 * \param motor Motor handle
 * \param revert TRUE to invert logic, FALSE otherwise
 */
void Motor_Invert(Motor_MotorDevice *motor, bool invert);
#endif


/*!
 * \brief Function to get a pointer to a motor (motor handle)
 * \param side Which motor
 * \return Pointer/handle to the motor
 */
Motor_MotorDevice *Motor_GetMotorHandle(Motor_MotorSide side);

/*!
 * \brief Changes the speed of a motor, in the range of -100% (backward) to +100% (forward).
 * \param motor Motor handle.
 * \param relPercent Relative speed percentage to change.
 */
void Motor_ChangeSpeedPercent(Motor_MotorDevice *motor, Motor_SpeedPercent relPercent);

/*!
 * \brief Returns the speed for a motor in percent (negative values are backward, positive are forward
 * \param motor Motor handle
 * \return The speed in percent, in the range -100...100
 */
Motor_SpeedPercent Motor_GetSpeedPercent(Motor_MotorDevice *motor);

/*!
 * \brief Sets the speed of a motor, in the range of -100% (backward) to +100% (forward).
 * \param motor Motor handle.
 * \param percent Motor speed value, from -100 (full speed backward) to +100 (full speed forward).
 */
void Motor_SetSpeedPercent(Motor_MotorDevice *motor, Motor_SpeedPercent percent);

/*!
 * \brief Updates the motor % speed based on actual PWM value.
 * \param motor Motor handle.
 * \param dir Current direction of motor.
 */
void Motor_UpdatePercent(Motor_MotorDevice *motor, Motor_Direction dir);

/*!
 * \brief Sets the PWM value for the motor.
 * \param[in] motor Motor handle
 * \param[in] val New PWM value.
 */
void Motor_SetVal(Motor_MotorDevice *motor, uint16_t val);

/*!
 * \brief Return the current PWM value of the motor.
 * \param[in] motor Motor handle
 * \return Current PWM value.
 */
uint16_t Motor_GetVal(Motor_MotorDevice *motor);

/*!
 * \brief Change the direction of the motor
 * \param[in] motor Motor handle
 * \param[in] dir Direction to be used
 */
void Motor_SetDirection(Motor_MotorDevice *motor, Motor_Direction dir);

/*!
 * \brief Returns the direction of the motor
 * \param[in] motor Motor handle
 * \return Current direction of the motor
 */
Motor_Direction Motor_GetDirection(Motor_MotorDevice *motor);

#if PL_CONFIG_USE_SHELL
#include "McuShell.h"
/*!
 * \brief Shell command line parser.
 * \param[in] cmd Pointer to command string
 * \param[out] handled If command is handled by the parser
 * \param[in] io Std I/O handler of shell
 */
uint8_t Motor_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif /* PL_CONFIG_USE_SHELL */

/*!
 * \brief Function to turn motors on/off, useful for debugging
 * \param on If motors shall be turned on or off. If turning off, the PWM is set to the 'off' level too.
 */
void Motor_OnOff(bool on);

/*!
 * \brief Deinitialization function.
 */
void Motor_Deinit(void);

/*!
 * \brief Initialization function.
 */
void Motor_Init(void);

#endif /* PL_CONFIG_USE_MOTORS */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* MOTOR_H_ */
