/**
 * \file
 * \brief Buzzer driver interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * This is the interface to the buzzer.
 */

#ifndef BUZZER_H_
#define BUZZER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"

#if PL_CONFIG_USE_BUZZER

#include "buzzer_config.h"
#include <stdint.h>

#if PL_CONFIG_USE_SHELL
  #include "McuShell.h"

/*!
 * \brief Shell parser routine.
 * \param cmd Pointer to command line string.
 * \param handled Pointer to status if command has been handled. Set to TRUE if command was understood.
 * \param io Pointer to stdio handle
 * \return Error code, ERR_OK if everything was ok.
 */
  uint8_t Buzzer_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif

/*!
 * \brief Let the buzzer sound for a specified time.
 * \param freqHz Frequency of the sound. Ignored if the buzzer is not supporting it.
 * \param durationMs Duration in milliseconds.
 * \return Error code, ERR_OK if everything is fine.
 */
uint8_t Buzzer_Beep(uint16_t freqHz, uint16_t durationMs);

typedef enum {
  BUZZER_TUNE_WELCOME,           /*!< Welcome/startup tune */
  BUZZER_TUNE_BUTTON,            /*!< Short button-press tone */
  BUZZER_TUNE_BUTTON_LONG,       /*!< Long button-press tone */
  BUZZER_TUNE_MAZE_DESTINATION,  /*!< Maze destination reached tune */
  BUZZER_TUNE_JINGLE_BELLS,      /*!< Jingle Bells melody */
  BUZZER_TUNE_NOF_TUNES          /*!< Sentinel: number of available tunes */
} Buzzer_Tunes;

/*!
 * \brief Plays a tune
 * \param tune Tune to play
 * \return ERR_OK or error code
 */
uint8_t Buzzer_PlayTune(Buzzer_Tunes tune);

/*!
 * \brief Stops any currently playing tune.
 */
void Buzzer_StopTune(void);

/*!
 * \brief Stops any currently active beep.
 */
void Buzzer_StopBeep(void);

/*!
 * \brief De-initialization of the driver
 */
void Buzzer_Deinit(void);

/*!
 * \brief Initialization of the driver
 */
void Buzzer_Init(void);

#endif /* PL_CONFIG_USE_BUZZER */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* BUZZER_H_ */
