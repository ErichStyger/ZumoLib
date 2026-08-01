#if 0
/**
 * \file
 * \brief Line sensor driver implementation.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 * This module implements a driver for the robot front line sensors.
 */

#include "platform.h"
#if PL_CONFIG_USE_REFLECTANCE
#include "lineSensor.h"
#include "McuUtility.h"
#include "McuGPIO.h"
#include "McuLED.h"
#include "McuLog.h"
#include "McuRTOS.h"

#define NOF_IR_LEDS            (5)
#define NOF_PHOTO_TRANSISTORS  (4)

static McuLED_Handle_t whiteLed;
static McuLED_Handle_t lineSensor_IR_LED[NOF_IR_LEDS]; /* left to right order */
static McuGPIO_Handle_t lineSensor_PhotoTransistor[NOF_PHOTO_TRANSISTORS]; /* left to right order */

static void LineSensorIRLeds(bool on) {
  for(int i=0;i<NOF_IR_LEDS;i++) {
    McuLED_Set(lineSensor_IR_LED[i], on);
  }
}

static int MeasureSensor(int idx) {
  #define MAX_COUNTER  (5000)
  int counter = MAX_COUNTER;
  McuGPIO_SetAsOutput(lineSensor_PhotoTransistor[idx], true); /* charge to high */
  vTaskDelay(pdMS_TO_TICKS(10)); /* give time to charge */
  portDISABLE_INTERRUPTS();
  McuGPIO_SetAsInput(lineSensor_PhotoTransistor[idx]); /* start discharging */
  /* turn on nearby IR LEDs */
  McuLED_On(lineSensor_IR_LED[idx]);
  McuLED_On(lineSensor_IR_LED[idx+1]);
  /* measure */
  for(counter=0; counter<MAX_COUNTER; counter++) {
    if (!McuGPIO_GetValue(lineSensor_PhotoTransistor[idx])) { /* low */
      break; /* leave for loop */
    }
  }  
  portENABLE_INTERRUPTS();
  /* turn IR off */
  McuLED_Off(lineSensor_IR_LED[idx]);
  McuLED_Off(lineSensor_IR_LED[idx+1]);
  return counter;
}


static void LineSensorScan(void) {
#if 1
  for (int i=0; i<NOF_PHOTO_TRANSISTORS; i++) {
    McuLog_info("val[%i]=%d", i, MeasureSensor(i));
  }
#else
  int data[NOF_PHOTO_TRANSISTORS];
  bool isLow[NOF_PHOTO_TRANSISTORS];
  int cntr, lowCntr;
  #define MAX_COUNTER  (5000)

  for (int i=0; i<NOF_PHOTO_TRANSISTORS; i++) {
    data[i] = MAX_COUNTER;
    isLow[i] = false;
  }
  /* charge to HIGH */
  for (int i=0; i<NOF_PHOTO_TRANSISTORS; i++) {
    McuGPIO_SetAsOutput(lineSensor_PhotoTransistor[i], true);
  }
  vTaskDelay(pdMS_TO_TICKS(50));
  portDISABLE_INTERRUPTS();
  /* discharge phase: input enabled */
  for (int i=0; i<NOF_PHOTO_TRANSISTORS; i++) {
    McuGPIO_SetAsInput(lineSensor_PhotoTransistor[i]);
  }
  LineSensorIRLeds(true); /* turn on IR LEDs */
  /* measure */
  for(cntr=0, lowCntr=0; cntr<5000 && lowCntr<NOF_PHOTO_TRANSISTORS; cntr++) {
    for (int i=0; i<NOF_PHOTO_TRANSISTORS; i++) {
      if (!isLow[i]) {
        if (!McuGPIO_GetValue(lineSensor_PhotoTransistor[i])) { /* low */
          data[i] = cntr;
          isLow[i] = true;
          lowCntr++;
        }
      }
    }
  }
  portENABLE_INTERRUPTS();
  LineSensorIRLeds(false);
  /* show data */
  for (int i=0; i<NOF_PHOTO_TRANSISTORS; i++) {
    McuLog_info("data[%i] = %d, low: %i", i, data[i], isLow[i]);
  }
#endif
}

static void PrintStatus(const McuShell_StdIOType *io) {
  unsigned char buf[96];

  McuShell_SendStatusStr((unsigned char*)"line", (unsigned char*)"Line sensor status\r\n", io->stdOut);
  McuLED_GetLedStatusString(whiteLed, buf, sizeof(buf));
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  white LED", buf, io->stdOut);
  for(int i=0; i<NOF_IR_LEDS; i++) {
    McuLED_GetLedStatusString(lineSensor_IR_LED[i], buf, sizeof(buf));
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
    McuShell_SendStatusStr((unsigned char*)"  IR LED", buf, io->stdOut); 
  }
  for(int i=0; i<NOF_PHOTO_TRANSISTORS; i++) {
    McuGPIO_GetPinStatusString(lineSensor_PhotoTransistor[i], buf, sizeof(buf));
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
    McuShell_SendStatusStr((unsigned char*)"  PT", buf, io->stdOut); 
  }
}

static void PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"line", (unsigned char*)"Group of line sensor commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows line sensor help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  white on|off", (unsigned char*)"Change white LED\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  ir on|off", (unsigned char*)"Change infrared LEDs\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  scan", (unsigned char*)"Do a line sensor scan\r\n", io->stdOut);
}

uint8_t LineSensor_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"line help")==0) {
    *handled = true;
    PrintHelp(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"line status")==0) {
    *handled = true;
    PrintStatus(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"line scan")==0) {
    *handled = true;
    LineSensorScan();
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"line white on")==0) {
    *handled = true;
    McuLED_On(whiteLed);
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"line white off")==0) {
    *handled = true;
    McuLED_Off(whiteLed);
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"line ir on")==0) {
    *handled = true;
    LineSensorIRLeds(true);
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"line ir off")==0) {
    *handled = true;
    LineSensorIRLeds(false);
    return ERR_OK;
  }
  return ERR_OK;
}

static void LineSensor_InitIRLedPins(void) {
  McuLED_Config_t ledConfig;

  LINE_SENSOR_CONFIG_ENABLE_IR_LED_CLOCK();
  McuLED_GetDefaultConfig(&ledConfig);
  ledConfig.isLowActive = true;
  ledConfig.hw.gpio = LINE_SENSOR_CONFIG_PINS_IR_PD_L_GPIO;
  ledConfig.hw.port = LINE_SENSOR_CONFIG_PINS_IR_PD_L_PORT;
  ledConfig.hw.pin = LINE_SENSOR_CONFIG_PINS_IR_PD_L_PIN;
  lineSensor_IR_LED[0] = McuLED_InitLed(&ledConfig);
  if (lineSensor_IR_LED[0]==NULL) {
    for(;;){}
  }
  ledConfig.hw.gpio = LINE_SENSOR_CONFIG_PINS_IR_PD_CL_GPIO;
  ledConfig.hw.port = LINE_SENSOR_CONFIG_PINS_IR_PD_CL_PORT;
  ledConfig.hw.pin = LINE_SENSOR_CONFIG_PINS_IR_PD_CL_PIN;
  lineSensor_IR_LED[1] = McuLED_InitLed(&ledConfig);
  if (lineSensor_IR_LED[1]==NULL) {
    for(;;){}
  }
  ledConfig.hw.gpio = LINE_SENSOR_CONFIG_PINS_IR_PD_C_GPIO;
  ledConfig.hw.port = LINE_SENSOR_CONFIG_PINS_IR_PD_C_PORT;
  ledConfig.hw.pin = LINE_SENSOR_CONFIG_PINS_IR_PD_C_PIN;
  lineSensor_IR_LED[2] = McuLED_InitLed(&ledConfig);
  if (lineSensor_IR_LED[2]==NULL) {
    for(;;){}
  }
  ledConfig.hw.gpio = LINE_SENSOR_CONFIG_PINS_IR_PD_CR_GPIO;
  ledConfig.hw.port = LINE_SENSOR_CONFIG_PINS_IR_PD_CR_PORT;
  ledConfig.hw.pin = LINE_SENSOR_CONFIG_PINS_IR_PD_CR_PIN;
  lineSensor_IR_LED[3] = McuLED_InitLed(&ledConfig);
  if (lineSensor_IR_LED[3]==NULL) {
    for(;;){}
  }
  ledConfig.hw.gpio = LINE_SENSOR_CONFIG_PINS_IR_PD_R_GPIO;
  ledConfig.hw.port = LINE_SENSOR_CONFIG_PINS_IR_PD_R_PORT;
  ledConfig.hw.pin = LINE_SENSOR_CONFIG_PINS_IR_PD_R_PIN;
  lineSensor_IR_LED[4] = McuLED_InitLed(&ledConfig);
  if (lineSensor_IR_LED[4]==NULL) {
    for(;;){}
  }
}

static void LineSensor_InitWhiteLedPin(void) {
  McuLED_Config_t ledConfig;

  LINE_SENSOR_CONFIG_ENABLE_IR_LED_CLOCK();
  McuLED_GetDefaultConfig(&ledConfig);
  ledConfig.isLowActive = true;
  ledConfig.hw.gpio = LINE_SENSOR_CONFIG_PINS_LED_WHITE_GPIO;
  ledConfig.hw.port = LINE_SENSOR_CONFIG_PINS_LED_WHITE_PORT;
  ledConfig.hw.pin = LINE_SENSOR_CONFIG_PINS_LED_WHITE_PIN;
  whiteLed = McuLED_InitLed(&ledConfig);
  if (whiteLed==NULL) {
    for(;;){}
  }
}

static void LineSensor_InitPhotoTransistorPins(void) {
  McuGPIO_Config_t config;

  LINE_SENSOR_CONFIG_ENABLE_PHOTO_TRANSISTOR_CLOCK();
  McuGPIO_GetDefaultConfig(&config);
  config.isInput = true;
  config.hw.pull = McuGPIO_PULL_DISABLE;
  config.hw.gpio = LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_L_GPIO;
  config.hw.port = LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_L_PORT;
  config.hw.pin = LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_L_PIN;
  lineSensor_PhotoTransistor[0] = McuGPIO_InitGPIO(&config);
  if (lineSensor_PhotoTransistor[0]==NULL) {
    for(;;){}
  }
  config.hw.gpio = LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_CL_GPIO;
  config.hw.port = LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_CL_PORT;
  config.hw.pin = LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_CL_PIN;
  lineSensor_PhotoTransistor[1] = McuGPIO_InitGPIO(&config);
  if (lineSensor_PhotoTransistor[1]==NULL) {
    for(;;){}
  }
  config.hw.gpio = LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_CR_GPIO;
  config.hw.port = LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_CR_PORT;
  config.hw.pin = LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_CR_PIN;
  lineSensor_PhotoTransistor[2] = McuGPIO_InitGPIO(&config);
  if (lineSensor_PhotoTransistor[2]==NULL) {
    for(;;){}
  }
  config.hw.gpio = LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_R_GPIO;
  config.hw.port = LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_R_PORT;
  config.hw.pin = LINE_SENSOR_CONFIG_PINS_PHOTO_TRANSISTOR_R_PIN;
  lineSensor_PhotoTransistor[3] = McuGPIO_InitGPIO(&config);
  if (lineSensor_PhotoTransistor[3]==NULL) {
    for(;;){}
  }
}

void LineSensor_Init(void) {
  LineSensor_InitIRLedPins();
  LineSensor_InitWhiteLedPin();
  LineSensor_InitPhotoTransistorPins();
}

#endif /* PL_CONFIG_USE_REFLECTANCE */

#endif