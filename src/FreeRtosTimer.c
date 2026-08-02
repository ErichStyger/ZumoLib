/*
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief FreeRTOS software timer module implementation.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */
#include "McuRTOS.h"
#if configUSE_TIMERS
#include "FreeRtosTimer.h"
#include "McuTimeDate.h"

static TimerHandle_t timerDateTime; /* timer for date/time */

static void vTimerCallbackDateTime(TimerHandle_t pxTimer) {
  McuTimeDate_AddTick();
}

void FreeRtosTimer_Init(void) {
  timerDateTime = xTimerCreate(
    "dateTimeTimer", /* name */
    pdMS_TO_TICKS(McuTimeDate_CONFIG_TICK_TIME_MS), /* period/time */
    pdTRUE, /* auto reload */
    (void*)0, /* timer ID */
    vTimerCallbackDateTime); /* callback */
  if (timerDateTime==NULL) {
    for(;;); /* failure! */
  }
  xTimerStart(timerDateTime, portMAX_DELAY);
}
#endif /* configUSE_TIMERS */
