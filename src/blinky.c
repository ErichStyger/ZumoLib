/*
 * Copyright 2026 Erich Styger
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief Blinky LED task implementation.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_BLINKY
#include "blinky.h"
#include "leds.h"
#include "McuGPIO.h"
#include "McuLED.h"
#include "McuLog.h"
#include "McuRTT.h"
#if McuLib_CONFIG_SDK_USE_FREERTOS
  #include "McuRTOS.h"
#endif
#if configUSE_PERCEPIO_TRACE_HOOKS
  #include "McuPercepio.h"
#endif

static void blinkyTask(void *pv) {
  for(;;) {
    Leds_On(LEDS_LEFT_RED);
    vTaskDelay(pdMS_TO_TICKS(1000));
    Leds_On(LEDS_RIGHT_RED);
    vTaskDelay(pdMS_TO_TICKS(1000));
    Leds_Off(LEDS_LEFT_RED);
    Leds_Off(LEDS_RIGHT_RED);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void Blinky_Init(void) {
  BaseType_t res = xTaskCreate(blinkyTask, "blinkyTask", 1*1024/sizeof(StackType_t), NULL, tskIDLE_PRIORITY, NULL);
  if (res!=pdPASS) {
    McuLog_fatal("failed creating blinky!");
  }
}
#endif /* PL_CONFIG_USE_BLINKY */