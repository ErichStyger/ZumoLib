/**
 * \file
 * \brief Reflectance sensor driver interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * This module implements a driver for the reflectance sensor array.
 */

#ifndef REFLECTANCE_H_
#define REFLECTANCE_H_

#include "platform.h"
#if PL_CONFIG_USE_REFLECTANCE

#include "refelectance_config.h"
#include <stdint.h>
#include <stdbool.h>

#if PL_CONFIG_USE_SHELL
  #include "McuShell.h"
  
  /*!
   * \brief Shell parser routine.
   * \param cmd Pointer to command line string.
   * \param handled Pointer to status if command has been handled. Set to TRUE if command was understood.
   * \param io Pointer to stdio handle
   * \return Error code, ERR_OK if everything was ok.
   */
  uint8_t REF_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);
#endif

#define REF_MIDDLE_LINE_VALUE  ((REFLECTANCE_CONFIG_NOF_SENSORS+1)*1000/2)
#define REF_MAX_LINE_VALUE     ((REFLECTANCE_CONFIG_NOF_SENSORS+1)*1000) /* maximum value for REF_GetLine() */

typedef enum {
  REF_LINE_NONE=0,     /* no line, sensors do not see a line */
  REF_LINE_STRAIGHT=1, /* forward line |, sensors see a line underneath */
  REF_LINE_LEFT=2,     /* left half of sensors see line */
  REF_LINE_RIGHT=3,    /* right half of sensors see line */
  REF_LINE_FULL=4,     /* all sensors see a line */
  REF_LINE_AIR=5,      /* all sensors have a timeout value. Robot is not on ground at all? */
  REF_NOF_LINES        /* Sentinel */
} REF_LineKind;


typedef enum {
  REF_LINE_KIND_MODE_LINE_FOLLOW,       /* returns REF_LINE_NONE, REF_LINE_STRAIGHT or REF_LINE_FULL */
  REF_LINE_KIND_MODE_ALL,               /* returns all different line kinds */
  REF_LINE_KIND_MODE_MAZE,              /* returns all different line kinds */
  REF_LINE_KIND_MODE_MAZE_LINE_FOLLOW,  /* used in maze mode, after stepping over the intersection */
  REF_LINE_KIND_MODE_SUMO,              /* returns all different line kinds */
} REF_LineKindMode;

/*!
 * \brief Determines the current line kind from calibrated sensor values.
 * \param mode Line detection mode.
 * \return Classified line kind.
 */
REF_LineKind REF_GetLineKind(REF_LineKindMode mode);

/*!
 * \brief Dumps calibrated reflectance values (debug output).
 */
void REF_DumpCalibrated(void);

/*!
 * \brief Returns a string representation for a line kind.
 * \param line Line kind to convert.
 * \return Pointer to static string.
 */
const unsigned char *REF_LineKindStr(REF_LineKind line);

/*!
 * \brief Returns normalized line position.
 * \param onLine Set to TRUE if a line is currently detected.
 * \return Normalized line position.
 */
uint16_t REF_GetLineValue(bool *onLine);
/*!
 * \brief Returns the detected line width.
 * \return Current line width estimate.
 */
uint16_t REF_LineWidth(void);

#if REFELECTANCE_CONFIG_DO_SENSOR_TRACING
  /*!
  * \brief Starts or stops RTT tracing for reflectance values.
  * \param start TRUE to start tracing, FALSE to stop.
  */
  void REF_StartStopRTTTrace(bool start);
#endif

/*!
 * \brief Reads raw reflectance sensor values.
 * \param values Destination array with REFLECTANCE_CONFIG_NOF_SENSORS entries.
 */
void REF_GetSensorRawValues(uint16_t values[REFLECTANCE_CONFIG_NOF_SENSORS]);

/*!
 * \brief Reads calibrated reflectance sensor values.
 * \param values Destination array.
 * \param nofValues Number of elements available in \a values.
 */
void REF_GetSensorValues(uint16_t *values, int nofValues);

/*!
 * \brief Starts or stops the calibration.
 */
void REF_CalibrateStartStop(void);

/*!
 * \brief Function to find out if we can use the sensor (means: it is calibrated and not currently calibrating)
 * \return TRUE if the sensor is ready.
 */
bool REF_CanUseSensor(void);

/*!
 * \brief Enable sensor handling and sensor task.
 */
void Reflectance_Enable(void);

/*!
 * \brief Disable sensor handling and sensor task.
 */
void Reflectance_Disable(void);

/*!
 * \brief Driver Deinitialization.
 */
void REF_Deinit(void);

/*!
 * \brief Driver Initialization.
 */
void REF_Init(void);

#endif /* PL_CONFIG_USE_REFLECTANCE */

#endif /* REFLECTANCE_H_ */
