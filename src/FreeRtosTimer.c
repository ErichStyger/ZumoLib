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

#include "platform.h"
#include "McuRTOS.h"
#if configUSE_TIMERS
#include "FreeRtosTimer.h"
#include "McuTimeDate.h"
#include "McuTimeout.h"

#if PL_CONFIG_USE_NEO_PIXEL_HW
static TimerHandle_t timerTimeout; /* timer for date/time */

static void vTimerCallbackTimeout(TimerHandle_t pxTimer) {
  McuTimeout_AddTick();
}
#endif

#if PL_CONFIG_USE_TIME_DATE
static TimerHandle_t timerDateTime; /* timer for date/time */

static void vTimerCallbackDateTime(TimerHandle_t pxTimer) {
  McuTimeDate_AddTick();
}
#endif

void FreeRtosTimer_Init(void) {
#if PL_CONFIG_USE_TIME_DATE
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
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  McuTimeout_Init();
  timerTimeout = xTimerCreate(
    "timeoutTimer", /* name */
    pdMS_TO_TICKS(McuTimeout_TICK_PERIOD_MS), /* period/time */
    pdTRUE, /* auto reload */
    (void*)0, /* timer ID */
    vTimerCallbackTimeout); /* callback */
  if (timerTimeout==NULL) {
    for(;;); /* failure! */
  }
  xTimerStart(timerTimeout, portMAX_DELAY);
#endif
}
#endif /* configUSE_TIMERS */
