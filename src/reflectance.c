/**
 * \file
 * \brief Reflectance sensor driver implementation.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * This module implements the driver for the bot front sensor.
 */
 
#include "platform.h"
#if PL_CONFIG_USE_REFLECTANCE
#include "reflectance.h"
#include "McuRTOS.h"
#include "McuGPIO.h"
#include "McuLED.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "McuWait.h"
#include "McuRTT.h"
#include "minIni/McuMinINI.h"
#if PL_CONFIG_USE_BUZZER
  #include "buzzer.h"
#endif
#include "application.h"
#include "shell.h"
#include "fsl_ftm.h"

/*------------------------------------------------------------------------------------------- */
/* Reflectance Sensor free running timer */
/*------------------------------------------------------------------------------------------- */
#define REFLECTANCE_FTM_BASEADDR        FTM1
#define REFLECTANCE_FTM_CLOCK_SRC       kFTM_SystemClock
#define REFLECTANCE_FTM_IRQ_NUM         FTM1_IRQn
#define REFLECTANCE_FTM_IRQ_HANDLER     FTM1_IRQHandler
#define REFLECTANCE_FTM_SOURCE_CLOCK    (CLOCK_GetFreq(kCLOCK_BusClk))
#define REFLECTANCE_FTM_INTERRUPT_ENABLE  (0) /* for testing only */

#if REFLECTANCE_FTM_INTERRUPT_ENABLE /* disabled, as no interrupt needed, only the counter */
void REFLECTANCE_FTM_IRQ_HANDLER(void) { /* called every 1 ms */
  /* Clear interrupt flag.*/
  FTM_ClearStatusFlags(REFLECTANCE_FTM_BASEADDR, kFTM_TimeOverflowFlag);
  __DSB();
}
#endif

void TMR_StartReflectanceTimer(void) {
  FTM_StartTimer(REFLECTANCE_FTM_BASEADDR, REFLECTANCE_FTM_CLOCK_SRC);
}

void TMR_StopReflectanceTimer(void) {
  FTM_StopTimer(REFLECTANCE_FTM_BASEADDR);
}

void TMR_ResetReflectanceTimerCounter(void) {
  REFLECTANCE_FTM_BASEADDR->CNT = 0; /* there is no SDK function for this */
}

uint32_t TMR_GetReflectanceTimerCounter(void) {
  return FTM_GetCurrentTimerCount(REFLECTANCE_FTM_BASEADDR);
}

uint32_t TMR_GetRefelectanceTimerFrequencyHz(void) {
  return REFLECTANCE_FTM_SOURCE_CLOCK;
}

static void InitReflectanceTimer(void) {
  ftm_config_t ftmInfo;

  FTM_GetDefaultConfig(&ftmInfo);
  /* Divide FTM clock by 4 */
  ftmInfo.prescale = kFTM_Prescale_Divide_1;

  /* Initialize FTM module */
  FTM_Init(REFLECTANCE_FTM_BASEADDR, &ftmInfo);
  FTM_SetTimerPeriod(REFLECTANCE_FTM_BASEADDR, 0xFFFF /*USEC_TO_COUNT(10000U), REFLECTANCE_FTM_SOURCE_CLOCK)*/);
#if REFLECTANCE_FTM_INTERRUPT_ENABLE
  FTM_EnableInterrupts(REFLECTANCE_FTM_BASEADDR, kFTM_TimeOverflowInterruptEnable);
  NVIC_SetPriority(REFLECTANCE_FTM_IRQ_NUM, 1);
  EnableIRQ(REFLECTANCE_FTM_IRQ_NUM);
#endif
}

typedef enum {
  REF_STATE_INIT,
  REF_STATE_NOT_CALIBRATED,
  REF_STATE_START_CALIBRATION,
  REF_STATE_CALIBRATING,
  REF_STATE_STOP_CALIBRATION,
  REF_STATE_SAVE_CALIBRATION,
  REF_STATE_READY
} RefStateType;

static volatile RefStateType refState = REF_STATE_INIT;

static struct {
  bool isEnabled; /* if sensor is enabled or not. If not enabled, it does not do any measurements */
  bool savePower; /* if true, the IR LEDs are turned off between measurements */
  void (*irOn)(bool); /* function pointer to turn on or off the IR LED(s) */
  McuGPIO_Handle_t sensor[REFLECTANCE_CONFIG_NOF_SENSORS]; /* pins for the IR sensor */
  McuLED_Handle_t ir[REFLECTANCE_CONFIG_NOF_IR]; /* pins for the IR sensor */
} REF_Sensor;
static SemaphoreHandle_t REF_StartStopSem = NULL;
static TaskHandle_t RefTaskHandle;

#define REF_TIMEOUT_TICKS   (((TMR_GetRefelectanceTimerFrequencyHz()/1000)*REF_SENSOR_TIMEOUT_US)/1000) /* REF_SENSOR_TIMEOUT_US translated into timeout ticks */

typedef uint16_t SensorTimeType;
#define MAX_SENSOR_VALUE  ((SensorTimeType)-1)

/* type of NVM Configuration data (in FLASH) */
typedef struct SensorCalibT_ {
  SensorTimeType minVal[REFLECTANCE_CONFIG_NOF_SENSORS];
  SensorTimeType maxVal[REFLECTANCE_CONFIG_NOF_SENSORS];
} SensorCalibT;

static SensorCalibT *SensorCalibMinMaxPtr; /* pointer to calibrated data, NULL if not calibrated */
static SensorCalibT SensorCalibMinMax; /* current calibration data */
static SensorCalibT *SensorCalibMinMaxTmpPtr=NULL; /* temporary calibration data */

static SensorTimeType SensorRaw[REFLECTANCE_CONFIG_NOF_SENSORS]; /* raw sensor values */
static SensorTimeType SensorCalibrated[REFLECTANCE_CONFIG_NOF_SENSORS]; /* 0 means white/min value, 1000 means black/max value */
static int16_t refCenterLineVal=0; /* 0 means no line, >0 means line is below sensor 0, 1000 below sensor 1 and so on */
static REF_LineKind refLineKind = REF_LINE_NONE;
static uint16_t refLineWidth = 0;

static uint8_t ReadIniReflectanceCalibration(SensorCalibT *data) {
  unsigned char buf[96];
  int res;
  const unsigned char *p;
  int32_t val;

  for(int i=0; i<REFLECTANCE_CONFIG_NOF_SENSORS; i++) { /* initialize values */
    data->minVal[i] = 0;
    data->maxVal[i] = 0; /* max value */
  }
  res = McuMinINI_ini_gets(REFLECTANCE_CONFIG_MININI_SECTION_REF_CALIBRATION, REFLECTANCE_CONFIG_MININI_KEY_CALIBRATION_MIN, "0 0 0 0 0 0", (char*)buf, sizeof(buf), REFLECTANCE_CONFIG_MININI_FILE_NAME);
  if (res==0) { /* nothing read? */
    return ERR_FAILED;
  }
  p = buf;
  for(int i=0; i<REFLECTANCE_CONFIG_NOF_SENSORS; i++) {
    if (McuUtility_xatoi(&p, &val)!=ERR_OK) {
      McuLog_error("failed to read min values from minINI");
      return ERR_FAILED;
    } else {
      data->minVal[i] = val;
    }
  }
  res = McuMinINI_ini_gets(REFLECTANCE_CONFIG_MININI_SECTION_REF_CALIBRATION, REFLECTANCE_CONFIG_MININI_KEY_CALIBRATION_MAX, "0 0 0 0 0 0", (char*)buf, sizeof(buf), REFLECTANCE_CONFIG_MININI_FILE_NAME);
  if (res==0) { /* nothing read? */
    return ERR_FAILED;
  }
  p = buf;
  for(int i=0; i<REFLECTANCE_CONFIG_NOF_SENSORS; i++) {
    if (McuUtility_xatoi(&p, &val)!=ERR_OK) {
      McuLog_error("failed to read max values from minINI");
      return ERR_FAILED;
    } else {
      data->maxVal[i] = val;
    }
  }
  return ERR_OK;
}

static uint8_t WriteIniReflectanceCalibration(const SensorCalibT *data) {
  unsigned char buf[96];

  buf[0] = '\0';
  for(int i=0; i<REFLECTANCE_CONFIG_NOF_SENSORS; i++) {
    McuUtility_strcatNum32s(buf, sizeof(buf), data->minVal[i]);
    if (i<REFLECTANCE_CONFIG_NOF_SENSORS-1) {
      McuUtility_chcat(buf, sizeof(buf), ' ');
    }
  }
  McuMinINI_ini_puts(REFLECTANCE_CONFIG_MININI_SECTION_REF_CALIBRATION, REFLECTANCE_CONFIG_MININI_KEY_CALIBRATION_MIN, (char*)buf, REFLECTANCE_CONFIG_MININI_FILE_NAME);
  buf[0] = '\0';
  for(int i=0; i<REFLECTANCE_CONFIG_NOF_SENSORS; i++) {
    McuUtility_strcatNum32s(buf, sizeof(buf), data->maxVal[i]);
    if (i<REFLECTANCE_CONFIG_NOF_SENSORS-1) {
      McuUtility_chcat(buf, sizeof(buf), ' ');
    }
  }
  McuMinINI_ini_puts(REFLECTANCE_CONFIG_MININI_SECTION_REF_CALIBRATION, REFLECTANCE_CONFIG_MININI_KEY_CALIBRATION_MAX, (char*)buf, REFLECTANCE_CONFIG_MININI_FILE_NAME);
  return ERR_OK;
}

void REF_GetSensorRawValues(uint16_t values[REFLECTANCE_CONFIG_NOF_SENSORS]) {
  memcpy(values, SensorRaw, REFLECTANCE_CONFIG_NOF_SENSORS*sizeof(SensorRaw[0]));
}

void REF_GetSensorValues(uint16_t *values, int nofValues) {
  int i;

  for(i=0;i<nofValues && i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
    values[i] = SensorCalibrated[i];
  }
}

/*!
 * \brief Measures the time until the sensor discharges
 * \param raw Array to store the raw values.
 * \return ERR_OVERFLOW if there is a timeout, ERR_OK otherwise
 */
static bool REF_MeasureRaw(SensorTimeType raw[REFLECTANCE_CONFIG_NOF_SENSORS], uint32_t timeoutCntVal) {
  uint8_t cnt; /* number of sensor */
  uint8_t i;
  uint32_t timerVal;

  if (!REF_Sensor.isEnabled) {
    return ERR_DISABLED;
  }
  if (REF_Sensor.savePower) {
    REF_Sensor.irOn(true); /* turn IR LED's on */
    McuWait_Waitus(200);
  }
  for(i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
    McuGPIO_SetAsOutput(REF_Sensor.sensor[i], true); /* turn I/O line as output */
    McuGPIO_SetHigh(REF_Sensor.sensor[i]);
    raw[i] = timeoutCntVal;
  }
  McuWait_Waitus(50); /* give at least 10 us to charge the capacitor */
  taskENTER_CRITICAL();
  for(i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
    McuGPIO_SetAsInput(REF_Sensor.sensor[i]); /* turn I/O line as input */
  }
  TMR_ResetReflectanceTimerCounter();
  TMR_StartReflectanceTimer(); /* start counting */
  do {
    timerVal = TMR_GetReflectanceTimerCounter();
    if (timerVal>timeoutCntVal) {
      break; /* get out of do-while loop */
    }
    cnt = 0;
    for(i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) { /* check all sensor if they are low */
      if (raw[i]==timeoutCntVal) { /* not low yet? */
        if (McuGPIO_IsLow(REF_Sensor.sensor[i])) { /* now it is low */
          raw[i] = timerVal; /* store the value */
          cnt++; /* count it as finished */
        }
      } else { /* already have value: count it */
        cnt++; /* count it as finished */
      }
    }
  } while(cnt!=REFLECTANCE_CONFIG_NOF_SENSORS); /* wait until timeout or all sensors are LOW (see 'white') */
  taskEXIT_CRITICAL();
  TMR_StopReflectanceTimer();
  if (REF_Sensor.savePower) {
    REF_Sensor.irOn(false); /* turn IR LED's off */
  }
  return ERR_OK;
}

static void REF_CalibrateMinMax(SensorTimeType min[REFLECTANCE_CONFIG_NOF_SENSORS], SensorTimeType max[REFLECTANCE_CONFIG_NOF_SENSORS], SensorTimeType raw[REFLECTANCE_CONFIG_NOF_SENSORS]) {
  int i;

  if (REF_MeasureRaw(raw, REF_TIMEOUT_TICKS)==ERR_OK) { /* if timeout, do not count values */
    for(i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
      if (raw[i] < min[i]) {
        min[i] = raw[i];
      }
      if (raw[i]> max[i]) {
        max[i] = raw[i];
      }
    }
  }
}

static void ReadCalibrated(SensorTimeType calib[REFLECTANCE_CONFIG_NOF_SENSORS], SensorTimeType raw[REFLECTANCE_CONFIG_NOF_SENSORS]) {
  int i;
  int32_t x, denominator;
  uint32_t max;
  
  max = 0; /* determine maximum timeout value */
  if (SensorCalibMinMaxPtr!=NULL) {
    for(i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
      if (SensorCalibMinMaxPtr->maxVal[i]>max) {
        max = SensorCalibMinMaxPtr->maxVal[i];
      }
    }
  }
  if (max > REF_TIMEOUT_TICKS) { /* limit measurement to timeout value */
    max = REF_TIMEOUT_TICKS;
  }
  (void)REF_MeasureRaw(raw, max);
  if (SensorCalibMinMaxPtr==NULL) { /* no calibration data? */
    return;
  }
  for(i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
    x = 0;
    denominator = SensorCalibMinMaxPtr->maxVal[i]-SensorCalibMinMaxPtr->minVal[i];
    if (denominator!=0) {
      x = (((int32_t)raw[i]-SensorCalibMinMaxPtr->minVal[i])*1000)/denominator;
    }
    if (x<0) {
      x = 0;
    } else if (x>1000) {
      x = 1000;
    }
    calib[i] = x;
  }
}

/*
 * Operates the same as read calibrated, but also returns an
 * estimated position of the robot with respect to a line. The
 * estimate is made using a weighted average of the sensor indices
 * multiplied by 1000, so that a return value of 1000 indicates that
 * the line is directly below sensor 0, a return value of 2000
 * indicates that the line is directly below sensor 1, 2000
 * indicates that it's below sensor 2000, etc. Intermediate
 * values indicate that the line is between two sensors. The
 * formula is:
 *
 * 1000*value0 + 2000*value1 + 3000*value2 + ...
 * --------------------------------------------
 * value0 + value1 + value2 + ...
 *
 * By default, this function assumes a dark line (high values)
 * surrounded by white (low values). If your line is light on
 * black, set the optional second argument white_line to true. In
 * this case, each sensor value will be replaced by (1000-value)
 * before the averaging.
 */
#define RETURN_LAST_VALUE  0
static int ReadLine(SensorTimeType calib[REFLECTANCE_CONFIG_NOF_SENSORS], SensorTimeType raw[REFLECTANCE_CONFIG_NOF_SENSORS], bool white_line) {
  int i;
  unsigned long avg; /* this is for the weighted total, which is long */
  /* before division */
  unsigned int sum; /* this is for the denominator which is <= 64000 */
  unsigned int mul; /* multiplication factor, 0, 1000, 2000, 3000 ... */
  int value;
#if RETURN_LAST_VALUE  
  bool on_line = FALSE;
  static int last_value=0; /* assume initially that the line is left. */
#endif
  
  (void)raw; /* unused */
  avg = 0;
  sum = 0;
  mul = 1000;
  for(i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
    value = calib[i];
    if(white_line) {
      value = 1000-value;
    }
    /* only average in values that are above a noise threshold */
    if(value > REF_MIN_NOISE_VAL) {
      avg += ((long)value)*mul;
      sum += value;
    }
    mul += 1000;
  }
#if RETURN_LAST_VALUE
  /* keep track of whether we see the line at all */
  if(value > REF_MIN_LINE_VAL) {
    on_line = TRUE;
  }
  if(!on_line) { /* If it last read to the left of center, return 0. */
    if(last_value < endIdx*1000/2) {
      return 0;
    } else { /* If it last read to the right of center, return the max. */
      return endIdx*1000;
    }
  }
  last_value = avg/sum;
  return last_value;
#else
  if (sum>0) {
    return avg/sum;
  } else {
    return avg;
  }
#endif
}

const unsigned char *REF_LineKindStr(REF_LineKind line) {
  switch(line) {
  case REF_LINE_NONE:
    return (unsigned char *)"NONE";
  case REF_LINE_STRAIGHT:
    return (unsigned char *)"STRAIGHT";
  case REF_LINE_LEFT:
    return (unsigned char *)"LEFT";
  case REF_LINE_RIGHT:
    return (unsigned char *)"RIGHT";
  case REF_LINE_FULL:
    return (unsigned char *)"FULL";
  case REF_LINE_AIR:
    return (unsigned char *)"AIR";
  default:
    return (unsigned char *)"unknown";
  } /* switch */
}

REF_LineKind REF_GetLineKind(REF_LineKindMode mode) {
  if (mode==REF_LINE_KIND_MODE_ALL || mode==REF_LINE_KIND_MODE_MAZE || mode==REF_LINE_KIND_MODE_SUMO) {
    return refLineKind;
  } else if (mode==REF_LINE_KIND_MODE_MAZE_LINE_FOLLOW) {
    switch(refLineKind) {
      case REF_LINE_NONE:
        return REF_LINE_NONE;

      case REF_LINE_LEFT:
      case REF_LINE_RIGHT:
      case REF_LINE_STRAIGHT:
        return REF_LINE_STRAIGHT;

      case REF_LINE_FULL:
        return REF_LINE_FULL;

      case REF_LINE_AIR:
      default:
        return REF_LINE_NONE;
    }
  } else if (mode==REF_LINE_KIND_MODE_LINE_FOLLOW) {
    switch(refLineKind) {
      case REF_LINE_NONE:
        return REF_LINE_NONE;

      case REF_LINE_LEFT:
      case REF_LINE_RIGHT:
      case REF_LINE_STRAIGHT:
        return REF_LINE_STRAIGHT;

      case REF_LINE_AIR:
      case REF_LINE_FULL:
        return REF_LINE_NONE;

      default:
        return REF_LINE_NONE;
    }
  }
  return refLineKind;
}

static bool SensorsSaturated(void) {
#if 0
  int i, cnt;
  
  /* check if robot is in the air or does see something */
  cnt = 0;
  for(i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
    if (SensorRaw[i]==MAX_SENSOR_VALUE) { /* sensor not seeing anything? */
      cnt++;
    }
  }
  return (cnt==REFLECTANCE_CONFIG_NOF_SENSORS); /* all sensors see raw max value: not on the ground? */
#else
  return FALSE;
#endif
}

static REF_LineKind CalculateLineKind(SensorTimeType val[REFLECTANCE_CONFIG_NOF_SENSORS]) {
  uint32_t sum, sumLeft, sumRight, outerLeft, outerRight, nofLines;
  int i;
  
  /* check if robot is in the air or does see something */
  if (SensorsSaturated()) {
    return REF_LINE_AIR;
  }
  for(i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
    if (val[i]<REF_MIN_LINE_VAL) { /* smaller value? White seen! */
      break;
    }
  }
  if (i==REFLECTANCE_CONFIG_NOF_SENSORS) { /* all sensors see 'black' */
    return REF_LINE_FULL;
  }
  /* check the line type */
  sum = 0; sumLeft = 0; sumRight = 0; nofLines = 0;
  for(i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
    if (val[i]>REF_MIN_LINE_VAL) { /* count only line values */
      nofLines++;
      sum += val[i];
      if (i<REFLECTANCE_CONFIG_NOF_SENSORS/2) {
        sumLeft += val[i];
      } else {
        sumRight += val[i];
      }
    }
  }
  outerLeft = val[0];
  outerRight = val[REFLECTANCE_CONFIG_NOF_SENSORS-1];
  
  #define MIN_LEFT_RIGHT_SUM   ((((REFLECTANCE_CONFIG_NOF_SENSORS)*2)/3)*REF_MIN_LINE_VAL) /* 2/3 of half the number of the sensors shall see a line */
  
  if (outerLeft<REF_MIN_LINE_VAL && outerRight<REF_MIN_LINE_VAL && nofLines>0) {
    /* both leftmost and rightmost sensors are seeing white, middle is seeing at least one black line */
    return REF_LINE_STRAIGHT; /* straight line forward */
  }
  if (outerLeft>=REF_MIN_LINE_VAL && outerRight<REF_MIN_LINE_VAL && sumLeft>MIN_LEFT_RIGHT_SUM && sumRight<MIN_LEFT_RIGHT_SUM) {
    return REF_LINE_LEFT; /* line going to the left side */
  } else if (outerLeft<REF_MIN_LINE_VAL && outerRight>=REF_MIN_LINE_VAL && sumRight>MIN_LEFT_RIGHT_SUM && sumLeft<MIN_LEFT_RIGHT_SUM) {
    return REF_LINE_RIGHT; /* line going to the right side */
  } else if (outerLeft>=REF_MIN_LINE_VAL && outerRight>=REF_MIN_LINE_VAL && sumRight>MIN_LEFT_RIGHT_SUM && sumLeft>MIN_LEFT_RIGHT_SUM) {
    return REF_LINE_FULL; /* full line */
  } else if (sumRight==0 && sumLeft==0 && sum == 0) {
    return REF_LINE_NONE; /* no line */
  } else { 
    return REF_LINE_STRAIGHT; /* straight line forward */
  }
}

uint16_t REF_LineWidth(void) {
  return refLineWidth;
}

static uint16_t CalculateRefLineWidth(SensorTimeType calib[REFLECTANCE_CONFIG_NOF_SENSORS]) {
  int32_t val;
  int i;

  val = 0;
  for(i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
    if (calib[i]>=REF_MIN_NOISE_VAL) { /* sensor not seeing anything? */
      val += calib[i];
    }
  }
  return (uint16_t)val;
}

#if REFELECTANCE_CONFIG_DO_SENSOR_TRACING
static bool REF_RTTtrace = false;
void REF_StartStopRTTTrace(bool start) {
  REF_RTTtrace = start;
}
#endif /* REFELECTANCE_CONFIG_DO_SENSOR_TRACING */

static void REF_Measure(void) {
  ReadCalibrated(SensorCalibrated, SensorRaw);
  refCenterLineVal = ReadLine(SensorCalibrated, SensorRaw, REF_USE_WHITE_LINE);
  refLineWidth = CalculateRefLineWidth(SensorCalibrated);
  if (refState==REF_STATE_READY) {
    refLineKind = CalculateLineKind(SensorCalibrated);
  }
#if REFELECTANCE_CONFIG_DO_SENSOR_TRACING
  if (REF_RTTtrace) {
    char buf[72];
    buf[0] = '\0';
    for(int i=0; i<REFLECTANCE_CONFIG_NOF_SENSORS; i++) {
      McuUtility_strcatNum16Hex((unsigned char*)buf, sizeof(buf), SensorRaw[i]);
      McuUtility_chcat((unsigned char*)buf, sizeof(buf), (unsigned char)' ' );
    }
    McuUtility_chcat((unsigned char*)buf, sizeof(buf), (unsigned char)':');
    for(int i=0; i<REFLECTANCE_CONFIG_NOF_SENSORS; i++) {
      McuUtility_strcatNum16Hex((unsigned char*)buf, sizeof(buf), SensorCalibrated[i]);
      McuUtility_chcat((unsigned char*)buf, sizeof(buf), (unsigned char)' ' );
    }
    McuUtility_strcat((unsigned char*)buf, sizeof(buf), REF_LineKindStr(refLineKind));
    McuUtility_chcat((unsigned char*)buf, sizeof(buf), (unsigned char)'\n');
    McuRTT_WriteString(0, buf);
  }
#endif /* REFELECTANCE_CONFIG_DO_SENSOR_TRACING */
}

void REF_CalibrateStartStop(void) {
  REF_Sensor.isEnabled = TRUE;
  REF_Sensor.irOn(true); /* turn IR LED's on */
  if (refState==REF_STATE_NOT_CALIBRATED || refState==REF_STATE_CALIBRATING || refState==REF_STATE_READY) {
    (void)xSemaphoreGive(REF_StartStopSem);
  }
}

bool REF_CanUseSensor(void) {
  return refState==REF_STATE_READY;
}

uint16_t REF_GetLineValue(bool *onLine) {
  *onLine = refCenterLineVal>0 && refCenterLineVal<REF_MAX_LINE_VALUE;
  return refCenterLineVal;
}

#if PL_CONFIG_USE_SHELL
static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"ref", (unsigned char*)"Group of Reflectance commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  enable|disable", (unsigned char*)"Enables or disables the reflectance measurement\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  calib (start|stop)", (unsigned char*)"Start/Stop calibrating while moving sensor over line\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  save power (on|off)", (unsigned char*)"Saves power with turning off IR between measurements\r\n", io->stdOut);
  return ERR_OK;
}

static unsigned char*REF_GetStateString(void) {
  switch (refState) {
    case REF_STATE_INIT:                return (unsigned char*)"INIT";
    case REF_STATE_NOT_CALIBRATED:      return (unsigned char*)"NOT CALIBRATED";
    case REF_STATE_START_CALIBRATION:   return (unsigned char*)"START CALIBRATION";
    case REF_STATE_CALIBRATING:         return (unsigned char*)"CALIBRATING";
    case REF_STATE_STOP_CALIBRATION:    return (unsigned char*)"STOP CALIBRATION";
    case REF_STATE_SAVE_CALIBRATION:    return (unsigned char*)"SAVE CALIBRATION";
    case REF_STATE_READY:               return (unsigned char*)"READY";
    default:
      break;
  } /* switch */
  return (unsigned char*)"UNKNOWN";
}

static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  unsigned char buf[32];
  int i;

  McuShell_SendStatusStr((unsigned char*)"reflectance", (unsigned char*)"IR line sensor status\r\n", io->stdOut);
  
  McuShell_SendStatusStr((unsigned char*)"  enabled", REF_Sensor.isEnabled?(unsigned char*)"yes\r\n":(unsigned char*)"no\r\n", io->stdOut);
#if REF_USE_WHITE_LINE
  McuShell_SendStatusStr((unsigned char*)"  line", (unsigned char*)"white\r\n", io->stdOut);
#else
  McuShell_SendStatusStr((unsigned char*)"  line", (unsigned char*)"black\r\n", io->stdOut);
#endif
  McuShell_SendStatusStr((unsigned char*)"  state", REF_GetStateString(), io->stdOut);
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  save power", REF_Sensor.savePower?(unsigned char*)"yes\r\n":(unsigned char*)"no\r\n", io->stdOut);

  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"0x");
  McuUtility_strcatNum16Hex(buf, sizeof(buf), REF_MIN_NOISE_VAL);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  min noise", buf, io->stdOut);

  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"0x");
  McuUtility_strcatNum16Hex(buf, sizeof(buf), REF_MIN_LINE_VAL);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  min line", buf, io->stdOut);
  
  McuUtility_Num16uToStr(buf, sizeof(buf), REF_SENSOR_TIMEOUT_US);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" us, 0x");
  McuUtility_strcatNum16Hex(buf, sizeof(buf), REF_TIMEOUT_TICKS);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" ticks\r\n");
  McuShell_SendStatusStr((unsigned char*)"  timeout", buf, io->stdOut);

  McuShell_SendStatusStr((unsigned char*)"  raw val", (unsigned char*)"", io->stdOut);
  for (i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
    if (i==0) {
      McuShell_SendStr((unsigned char*)"0x", io->stdOut);
    } else {
      McuShell_SendStr((unsigned char*)" 0x", io->stdOut);
    }
    buf[0] = '\0'; McuUtility_strcatNum16Hex(buf, sizeof(buf), SensorRaw[i]);
    McuShell_SendStr(buf, io->stdOut);
  }
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  
  if (SensorCalibMinMaxPtr!=NULL) { /* have calibration data */
    McuShell_SendStatusStr((unsigned char*)"  min val", (unsigned char*)"", io->stdOut);
    for (i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
      if (i==0) {
        McuShell_SendStr((unsigned char*)"0x", io->stdOut);
      } else {
        McuShell_SendStr((unsigned char*)" 0x", io->stdOut);
      }
      buf[0] = '\0'; McuUtility_strcatNum16Hex(buf, sizeof(buf), SensorCalibMinMaxPtr->minVal[i]);
      McuShell_SendStr(buf, io->stdOut);
    } /* for */
    McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  }
  if (SensorCalibMinMaxPtr!=NULL) {
    McuShell_SendStatusStr((unsigned char*)"  max val", (unsigned char*)"", io->stdOut);
    for (i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
      if (i==0) {
        McuShell_SendStr((unsigned char*)"0x", io->stdOut);
      } else {
        McuShell_SendStr((unsigned char*)" 0x", io->stdOut);
      }
      buf[0] = '\0'; McuUtility_strcatNum16Hex(buf, sizeof(buf), SensorCalibMinMaxPtr->maxVal[i]);
      McuShell_SendStr(buf, io->stdOut);
    }
    McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  }

  if (SensorCalibMinMaxPtr!=NULL) { /* have calibration data */
    McuShell_SendStatusStr((unsigned char*)"  calib val", (unsigned char*)"", io->stdOut);
    for (i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
      if (i==0) {
        McuShell_SendStr((unsigned char*)"0x", io->stdOut);
      } else {
        McuShell_SendStr((unsigned char*)" 0x", io->stdOut);
      }
      buf[0] = '\0'; McuUtility_strcatNum16Hex(buf, sizeof(buf), SensorCalibrated[i]);
      McuShell_SendStr(buf, io->stdOut);
    }
    McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  }
  
  McuShell_SendStatusStr((unsigned char*)"  line pos", (unsigned char*)"", io->stdOut);
  buf[0] = '\0'; McuUtility_strcatNum16s(buf, sizeof(buf), refCenterLineVal);
  McuShell_SendStr(buf, io->stdOut);
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);

  McuShell_SendStatusStr((unsigned char*)"  line width", (unsigned char*)"", io->stdOut);
  buf[0] = '\0'; McuUtility_strcatNum16s(buf, sizeof(buf), refLineWidth);
  McuShell_SendStr(buf, io->stdOut);
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);

  McuShell_SendStatusStr((unsigned char*)"  line kind", REF_LineKindStr(refLineKind), io->stdOut);
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  return ERR_OK;
}
#endif /* PL_CONFIG_USE_SHELL */

void Reflectance_Enable(void) {
  REF_Sensor.isEnabled = TRUE;
  REF_Sensor.irOn(true); /* turn IR LED's on */
  vTaskResume(RefTaskHandle);
}

void Reflectance_Disable(void) {
  REF_Sensor.isEnabled = FALSE;
  REF_Sensor.irOn(false); /* turn IR LED's off */
  vTaskSuspend(RefTaskHandle);
}

#if PL_CONFIG_USE_SHELL
uint8_t REF_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "ref help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "ref status")==0)) {
    *handled = TRUE;
    return PrintStatus(io);
  } else if (McuUtility_strcmp((char*)cmd, "ref enable")==0) {
    *handled = TRUE;
    Reflectance_Enable();
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, "ref disable")==0) {
    *handled = TRUE;
    Reflectance_Disable();
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, "ref calib start")==0) {
    if (refState==REF_STATE_NOT_CALIBRATED || refState==REF_STATE_READY) {
      APP_StateStartCalibrate();
    } else {
      McuShell_SendStr((unsigned char*)"ERROR: cannot start calibration, must not be calibrating or be ready.\r\n", io->stdErr);
      return ERR_FAILED;
    }
    *handled = TRUE;
    return ERR_OK;  
  } else if (McuUtility_strcmp((char*)cmd, "ref calib stop")==0) {
    if (refState==REF_STATE_CALIBRATING) {
      APP_StateStopCalibrate();
    } else {
      McuShell_SendStr((unsigned char*)"ERROR: can only stop if calibrating.\r\n", io->stdErr);
      return ERR_FAILED;
    }
    *handled = TRUE;
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, "ref save power on")==0) {
    REF_Sensor.savePower = true;
    REF_Sensor.irOn(false); /* turn IR LED's off */
    *handled = TRUE;
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, "ref save power off")==0) {
    REF_Sensor.savePower = false;
    *handled = TRUE;
    return ERR_OK;
  }
  return ERR_OK;
}
#endif /* PL_CONFIG_USE_SHELL */

static void REF_StateMachine(void) {
  int i;

  switch (refState) {
    case REF_STATE_INIT:
      if (SensorCalibMinMaxPtr==NULL) { /* not loaded yet? */
        SensorCalibT calib;
        if (ReadIniReflectanceCalibration(&calib)==ERR_OK) {
          SensorCalibMinMax = calib; /* struct copy */
          SensorCalibMinMaxPtr = &SensorCalibMinMax;
        }
      }
      if (SensorCalibMinMaxPtr!=NULL) { /* use existing calibration data */
        refState = REF_STATE_READY;
      } else {
        McuLog_warn("no calibration data present");
        refState = REF_STATE_NOT_CALIBRATED;
      }
      break;
      
    case REF_STATE_NOT_CALIBRATED:
      vTaskDelay(pdMS_TO_TICKS(80)); /* no need to sample that fast: this gives 80+20=100 ms */
      (void)REF_MeasureRaw(SensorRaw, REF_TIMEOUT_TICKS);
      if (xSemaphoreTake(REF_StartStopSem, 0)==pdTRUE) {
        refState = REF_STATE_START_CALIBRATION;
      }
      break;
    
    case REF_STATE_START_CALIBRATION:
      McuLog_info("start calibration...");
      if (SensorCalibMinMaxTmpPtr!=NULL) {
        refState = REF_STATE_INIT; /* error case */
        break;
      }
      SensorCalibMinMaxTmpPtr = pvPortMalloc(sizeof(SensorCalibT));
      if (SensorCalibMinMaxTmpPtr!=NULL) { /* success */
        for(i=0;i<REFLECTANCE_CONFIG_NOF_SENSORS;i++) {
          SensorCalibMinMaxTmpPtr->minVal[i] = MAX_SENSOR_VALUE;
          SensorCalibMinMaxTmpPtr->maxVal[i] = 0;
          SensorCalibrated[i] = 0;
        }
        refState = REF_STATE_CALIBRATING;
      } else {
        refState = REF_STATE_INIT; /* error case */
      }
      break;
    
    case REF_STATE_CALIBRATING:
      vTaskDelay(pdMS_TO_TICKS(80)); /* no need to sample that fast: this gives 80+20=100 ms */
      if (SensorCalibMinMaxTmpPtr!=NULL) { /* safety check */
        REF_CalibrateMinMax(SensorCalibMinMaxTmpPtr->minVal, SensorCalibMinMaxTmpPtr->maxVal, SensorRaw);
      } else {
        McuLog_error("no calibration data?");
        refState = REF_STATE_INIT; /* error case */
        break;
      }
#if PL_CONFIG_USE_BUZZER
      (void)Buzzer_Beep(300, 50);
#endif
      if (xSemaphoreTake(REF_StartStopSem, 0)==pdTRUE) {
        refState = REF_STATE_STOP_CALIBRATION;
      }
      break;
    
    case REF_STATE_STOP_CALIBRATION:
#if PL_CONFIG_USE_SHELL
      McuLog_info("...stopping calibration.");
#endif
      refState = REF_STATE_SAVE_CALIBRATION;
      break;
    
    case REF_STATE_SAVE_CALIBRATION:
      SensorCalibMinMax = *SensorCalibMinMaxTmpPtr; /* struct copy */
      SensorCalibMinMaxPtr = &SensorCalibMinMax;
      if (WriteIniReflectanceCalibration(&SensorCalibMinMax)!=ERR_OK) {
        McuLog_fatal("FAILED storing to FLASH!?!");
      }
      /* free memory */
      vPortFree(SensorCalibMinMaxTmpPtr);
      SensorCalibMinMaxTmpPtr = NULL;
      if (SensorCalibMinMaxPtr!=NULL) { /* use calibration data we have available */
        McuLog_info("calibration data saved.");
        refState = REF_STATE_READY;
        break;
      } else {
        refState = REF_STATE_INIT; /* error case */
      }
      break;
    
    case REF_STATE_READY:
      REF_Measure();
      if (xSemaphoreTake(REF_StartStopSem, 0)==pdTRUE) {
        refState = REF_STATE_START_CALIBRATION;
      }
      break;
  } /* switch */
}

static void ReflTask(void *pvParameters) {
  (void)pvParameters; /* not used */
  for(;;) {
    if (!REF_Sensor.isEnabled) {
      vTaskSuspend(NULL);
    }
    REF_StateMachine();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

static void InitSensorPins(void) {
  McuGPIO_Config_t gpioConfig;
  McuGPIO_HwPin_t hw[REFLECTANCE_CONFIG_NOF_SENSORS] =
  {
    REFLECTANCE_CONFIG_SENSOR_PINS()
  };
  REFLECTANCE_CONFIG_ENABLE_SENSOR_CLOCK();
  McuGPIO_GetDefaultConfig(&gpioConfig);
  gpioConfig.isInput = true;
  gpioConfig.isHighOnInit = false;
  for(int i=0; i<REFLECTANCE_CONFIG_NOF_SENSORS; i++) {
    gpioConfig.hw = hw[i];
    REF_Sensor.sensor[i] = McuGPIO_InitGPIO(&gpioConfig);
  }
}

static void InitIRPins(void) {
  McuLED_Config_t config;
  McuGPIO_HwPin_t hw[REFLECTANCE_CONFIG_NOF_IR] =
  {
    REFLECTANCE_CONFIG_IR_PINS()
  };
  REFLECTANCE_CONFIG_ENABLE_IR_CLOCK();
  McuLED_GetDefaultConfig(&config);
  config.isLowActive = REFLECTANCE_CONFIG_IR_IS_LOW_ACTIVE;
  for(int i=0; i<REFLECTANCE_CONFIG_NOF_IR; i++) {
    config.hw = hw[i];
    REF_Sensor.ir[i] = McuLED_InitLed(&config);
  }
}

static void IrOnOff(bool on) {
  for(int i=0; i<REFLECTANCE_CONFIG_NOF_IR; i++) {
    McuLED_Set(REF_Sensor.ir[i], on);
  }
}

void REF_Init(void) {
  InitSensorPins();
  InitIRPins();
  REF_Sensor.isEnabled = true;

  REF_Sensor.irOn = IrOnOff;
  REF_Sensor.savePower = false; /* turn off, as this extends the ref sensor task time by around 200 us */
  if (REF_Sensor.savePower) {
    REF_Sensor.irOn(false); /* turn IR LED's off */
  } else {
    REF_Sensor.irOn(true); /* turn IR LED's on */
  }

  REF_Sensor.isEnabled = true;
  if (REF_Sensor.isEnabled) {
    REF_Sensor.irOn(true); /* turn IR LED's on */
  } else {
    REF_Sensor.irOn(false); /* turn IR LED's off */
  }
  vSemaphoreCreateBinary(REF_StartStopSem);
  if (REF_StartStopSem==NULL) { /* semaphore creation failed */
    for(;;){} /* error */
  }
  (void)xSemaphoreTake(REF_StartStopSem, 0); /* empty token */
  vQueueAddToRegistry(REF_StartStopSem, "RefStartStopSem");
#if configUSE_TRACE_HOOKS
  vTraceSetQueueName(REF_StartStopSem, "RefStartStopSem");
#endif
  refState = REF_STATE_INIT;
#if PL_REMOTE_STOP_LINE
  refLineKind = REF_LINE_FULL; /* need an initial state, assume on black so the robot does not move */
#else
  refLineKind = REF_LINE_NONE;
#endif
  refCenterLineVal = 0;
  if (xTaskCreate(ReflTask, "Refl", 1024/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+3, &RefTaskHandle) != pdPASS) {
    for(;;){} /* error */
  }
  InitReflectanceTimer();
}

#endif /* PL_CONFIG_USE_REFLECTANCE */
