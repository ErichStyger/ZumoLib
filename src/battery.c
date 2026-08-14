/**
 * \file
 * \brief Module for the battery management.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 *
 * Deals with the robot battery.
 */

#include "platform.h"
#if PL_CONFIG_HAS_BATTERY_ADC
#include "Battery.h"
#include "fsl_adc16.h"
#include "McuShell.h"
#include "McuRTOS.h"
#include "McuUtility.h"

#define BATT_ADC16_BASE          ADC1
#define BATT_ADC16_CHANNEL_GROUP 0U
#define BATT_ADC16_USER_CHANNEL  19U

static adc16_channel_config_t adc16ChannelConfigStruct;

uint8_t BATT_MeasureBatteryVoltage(uint16_t *cvP) {
  #define BAT_V_DIVIDER_UP   62 /* voltage divider pull-up */
  #define BAT_V_DIVIDER_DOWN 30 /* voltage divider pull-down */
  uint32_t milliVolts;
  uint16_t adcValue;

  ADC16_SetChannelConfig(BATT_ADC16_BASE, BATT_ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
  while (0U == (kADC16_ChannelConversionDoneFlag & ADC16_GetChannelStatusFlags(BATT_ADC16_BASE, BATT_ADC16_CHANNEL_GROUP)))
  {
    vTaskDelay(pdMS_TO_TICKS(1)); /* wait */
  }
  adcValue = ADC16_GetChannelConversionValue(BATT_ADC16_BASE, BATT_ADC16_CHANNEL_GROUP);
  /* reference voltage is 3.3V. Battery Voltage is using a voltage divider (R29, 62KOhm pullup to VBat, R30 30kOhm pull down to GND) */
  milliVolts = adcValue*330*(BAT_V_DIVIDER_UP+BAT_V_DIVIDER_DOWN)/BAT_V_DIVIDER_DOWN/0xffff; /* scale it to centi-volt. Do multiplication first to avoid numerical issues */
  *cvP = milliVolts;
  return ERR_OK;
}

static uint8_t BATT_PrintStatus(const McuShell_StdIOType *io) {
  uint8_t buf[32];
  uint16_t cv;

  McuShell_SendStatusStr((unsigned char*)"battery", (unsigned char*)"Battery status\r\n", io->stdOut);
  buf[0] = '\0';
  if (BATT_MeasureBatteryVoltage(&cv)==ERR_OK) {
    McuUtility_strcatNum32sDotValue100(buf, sizeof(buf), cv);
    McuUtility_strcat(buf, sizeof(buf), (uint8_t*)" V\r\n");
  } else {
    McuUtility_strcat(buf, sizeof(buf), (uint8_t*)"ERROR\r\n");
  }
  McuShell_SendStatusStr((unsigned char*)"  Battery", buf, io->stdOut);
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"battery", (unsigned char*)"Group of battery commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  return ERR_OK;
}

uint8_t BATT_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  uint8_t res = ERR_OK;

  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"battery help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"battery status")==0) {
    *handled = TRUE;
    return BATT_PrintStatus(io);
  }
  return res;
}


void BATT_Init(void) {
  adc16_config_t adc16ConfigStruct;
  /*
    * adc16ConfigStruct.referenceVoltageSource = kADC16_ReferenceVoltageSourceVref;
    * adc16ConfigStruct.clockSource = kADC16_ClockSourceAsynchronousClock;
    * adc16ConfigStruct.enableAsynchronousClock = true;
    * adc16ConfigStruct.clockDivider = kADC16_ClockDivider8;
    * adc16ConfigStruct.resolution = kADC16_ResolutionSE12Bit;
    * adc16ConfigStruct.longSampleMode = kADC16_LongSampleDisabled;
    * adc16ConfigStruct.enableHighSpeed = false;
    * adc16ConfigStruct.enableLowPower = false;
    * adc16ConfigStruct.enableContinuousConversion = false;
    */
   ADC16_GetDefaultConfig(&adc16ConfigStruct);

   adc16ConfigStruct.longSampleMode = kADC16_LongSampleCycle24;
   adc16ConfigStruct.resolution = kADC16_Resolution16Bit;

   ADC16_Init(BATT_ADC16_BASE, &adc16ConfigStruct);
   ADC16_EnableHardwareTrigger(BATT_ADC16_BASE, false); /* Make sure the software trigger is used. */
   if (kStatus_Success == ADC16_DoAutoCalibration(BATT_ADC16_BASE))
   {
       //PRINTF("ADC16_DoAutoCalibration() Done.\r\n");
   }
   else
   {
       //PRINTF("ADC16_DoAutoCalibration() Failed.\r\n");
     for(;;) {}
   }
   adc16ChannelConfigStruct.channelNumber = BATT_ADC16_USER_CHANNEL;
   adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = false;
#if defined(FSL_FEATURE_ADC16_HAS_DIFF_MODE) && FSL_FEATURE_ADC16_HAS_DIFF_MODE
   adc16ChannelConfigStruct.enableDifferentialConversion = false;
#endif /* FSL_FEATURE_ADC16_HAS_DIFF_MODE */

}

void BATT_Deinit(void) {
}

#endif /* PL_CONFIG_HAS_BATTERY_ADC */
