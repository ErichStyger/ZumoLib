/*
 * Copyright (c) 2023, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_SENSIRION
#include "sensirion.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "McuRTOS.h"
#include "McuGenericI2C.h"
#include "McuGDisplaySSD1306.h"
#include "McuFontDisplay.h"
#include "McuFontHelv18Bold.h"
#if PL_CONFIG_USE_SHT31
  #include "McuSHT31.h"
#elif PL_CONFIG_USE_SHT40
  #include "McuSHT40.h"
#else
  #error "unknown sensor?"
#endif
#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  #include "McuSystemView.h"
#endif

static float Sensirion_temperature, Sensirion_humidity;

float Sensirion_GetTemperature(void) {
  return Sensirion_temperature; /* technically no mutex required, as 32bit access in one instruction. But leave at least a comment. Mutex required if both sensor values need to be from the same time */
}

float Sensirion_GetHumidity(void) {
  return Sensirion_humidity; /* technically no mutex required, as 32bit access in one instruction */
}

static void sensirionTask(void *pv) {
  uint8_t res;
  float temperature, humidity;

  Sensirion_temperature = Sensirion_humidity = 0.0f; /* init */
#if PL_CONFIG_USE_SHT31
  McuSHT31_Init();
#elif PL_CONFIG_USE_SHT40
  McuSHT40_Init();
#endif
  for(;;) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    McuGenericI2C_RequestBus();
  #if PL_CONFIG_USE_SHT31
    res = McuSHT31_ReadTempHum(&temperature, &humidity);
  #elif PL_CONFIG_USE_SHT40
    res = McuSHT40_ReadTempHum(&temperature, &humidity);
  #endif
    McuGenericI2C_ReleaseBus();
    if (res==ERR_OK) {
      Sensirion_temperature = temperature;
      Sensirion_humidity = humidity;
    } else {
      McuLog_error("failed reading SHT sensor.");
      vTaskDelay(pdMS_TO_TICKS(5000));
    }
  }
}

void SensirionDeinit(void) {
}

void Sensirion_Init(void) {
  BaseType_t res;

  res = xTaskCreate(sensirionTask, "sensirion", (4*1024)/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+1, NULL);
  if (res!=pdPASS) {
    /* error! */
    for(;;) {}
  }
}
#endif /* PL_CONFIG_USE_SENSIRION */
