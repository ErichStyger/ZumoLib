/*
 * Copyright (c) 2021, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_REFLECTANCE
#include "lineHistory.h"
#include "reflectance.h"
#include "McuLog.h"

#define HISTORY_MIN_SENSOR_VAL      (1000/2)   /* minimum threshold for values recorded in history: calib values below this are 'white', and above this are 'black' */

static uint16_t SensorHistory[REFLECTANCE_CONFIG_NOF_SENSORS]; /* value of history while moving forward */

void HISTORY_SampleSensors(void) {
  uint8_t i;
  uint16_t val[REFLECTANCE_CONFIG_NOF_SENSORS];
#if 0
  uint16_t raw[REFLECTANCE_CONFIG_NOF_SENSORS];
  REF_GetSensorRawValues(raw);
  McuLog_trace("raw: %x %x %x %x %x %x", raw[0], raw[1], raw[2], raw[3], raw[4], raw[5]);
#endif
  REF_GetSensorValues(&val[0], REFLECTANCE_CONFIG_NOF_SENSORS);
  for(i=0; i<REFLECTANCE_CONFIG_NOF_SENSORS; i++) {
    if (val[i]>SensorHistory[i]) {
      SensorHistory[i] = val[i];
    }
  }
}

/*!
 * \brief Can be called during turning, will use it to sample sensor values.
 */
bool HISTORY_SampleTurnStopFunction(void) {
  HISTORY_SampleSensors();
  return FALSE; /* do not stop turning */
}

REF_LineKind HISTORY_LineKind(void) {
  int i, cnt, cntLeft, cntRight;

  cnt = cntLeft = cntRight = 0;
  for(i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
    if (SensorHistory[i]>=HISTORY_MIN_SENSOR_VAL) { /* count above threshold values */
      cnt++;
#if REF_SENSOR1_IS_LEFT
      if (i<REFLECTANCE_CONFIG_NOF_SENSORS/2) {
        cntLeft++;
      } else {
        cntRight++;
      }
#else
      if (i<REFLECTANCE_CONFIG_NOF_SENSORS/2) {
        cntRight++;
      } else {
        cntLeft++;
      }
#endif
    }
  }
  if (cnt==0) {
    return REF_LINE_NONE;
  } else if (cnt==REFLECTANCE_CONFIG_NOF_SENSORS) {
    return REF_LINE_FULL;
  } else if (SensorHistory[0]<HISTORY_MIN_SENSOR_VAL && SensorHistory[REFLECTANCE_CONFIG_NOF_SENSORS-1]<HISTORY_MIN_SENSOR_VAL) {
    return REF_LINE_STRAIGHT; /* there is still white to the left and right */
  } else if (SensorHistory[0]>=HISTORY_MIN_SENSOR_VAL) {
    return REF_LINE_LEFT;
  } else { /* must be cntRight>cntLeft */
    return REF_LINE_RIGHT;
  }
  return REF_LINE_NONE;
}

void HISTORY_Clear(void) {
  int i;

  for(i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
    SensorHistory[i] = 0;
  }
}
#endif /* PL_CONFIG_USE_REFLECTANCE */

