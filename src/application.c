/*
 * Copyright 2026 Erich Styger
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * \file
 * \brief Main application state machine implementation.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */
  
#include "application.h"
#include "platform.h"
#include "McuRTOS.h"
#include "McuLog.h"
#include "McuDebounce.h"
#include "buttons.h"
#if PL_CONFIG_USE_REFLECTANCE
  #include "reflectance.h"
#endif
#if PL_CONFIG_LINE_FOLLOWING
  #include "lineFollow.h"
#endif
#if PL_CONFIG_USE_I2C && MCU_LIS2DH_CONFIG_IS_ENABLED
  #include "McuLis2dh.h"
#endif
#if PL_CONFIG_USE_BUZZER
  #include "buzzer.h"
#endif

typedef enum {
  APP_STATE_STARTUP,
  APP_STATE_INIT,
#if PL_CONFIG_USE_REFLECTANCE
  APP_STATE_CALIBRATE,
#endif
#if PL_CONFIG_APP_LINE_FOLLOWING || PL_CONFIG_APP_LINE_MAZE
  APP_STATE_FOLLOW_LINE,
#endif
#if PL_APP_FOLLOW_OBSTACLE
  APP_STATE_FOLLOW_OBSTACLE, /* follow obstacle */
#endif
#if PL_DO_MIDI
  APP_STATE_PLAY_MIDI,
#endif
#if PL_CONFIG_APP_SUMO
  APP_STATE_RUN_SUMO, /* Sumo fight */
#endif
  APP_STATE_IDLE
} AppStateType;

static AppStateType appState = APP_STATE_STARTUP;

#if PL_CONFIG_USE_REFLECTANCE
bool Application_StateIsCalibrating(void) {
  return appState == APP_STATE_CALIBRATE;
}

void Application_StateStartCalibrate(void) {
  REF_CalibrateStartStop();
  appState = APP_STATE_CALIBRATE;
}

void Application_StateStopCalibrate(void) {
  appState = APP_STATE_IDLE;
  REF_CalibrateStartStop();
}
#endif /* PL_CONFIG_USE_REFLECTANCE */

#if PL_CONFIG_USE_DEBOUNCE
void Application_OnButtonEvent(Buttons_e button, McuDbnc_EventKinds event) { /* called from debouncing */
  //McuLog_info("on debounce event: button %d, event %d", button, event);
  if (button==BUTTONS_USER) {
    if (event==MCUDBNC_EVENT_RELEASED) {
      #if PL_CONFIG_APP_LINE_FOLLOWING
        LineFollow_StartStopFollowing(); /* button press toggles line following mode */   
        #if PL_CONFIG_USE_BUZZER 
        Buzzer_PlayTune(BUZZER_TUNE_BUTTON);
        #endif
      #endif
    }
  }
}
#endif /* PL_CONFIG_USE_DEBOUNCE */

#if PL_CONFIG_USE_APP_TASK

static AppStateType StateMachine(bool buttonPress, AppStateType currState) {
  switch (currState) {
    case APP_STATE_STARTUP:
    #if PL_CONFIG_USE_BUZZER 
      Buzzer_PlayTune(BUZZER_TUNE_WELCOME);
    #endif
      currState = APP_STATE_INIT;
      break;
    case APP_STATE_INIT:
      break;
    default:
      break;
  } /* switch */
  return currState;
}

static void AppTask(void *pv) {
#if PL_CONFIG_USE_I2C && MCU_LIS2DH_CONFIG_IS_ENABLED
  if (McuLis2dh_Init()!=ERR_OK) {
    McuLog_fatal("init I2C accelerometer failed");
    for(;;) {}
  }
#endif
  appState = APP_STATE_STARTUP;
  for(;;) {
    appState = StateMachine(false, appState);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
#endif

void Application_Init(void) {
#if PL_CONFIG_USE_APP_TASK
  if (xTaskCreate(AppTask, "App", 1024/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) {
    for(;;){} /* error */
  }
#endif
}

#if PL_CONFIG_IS_ROBOT
void Application_Run(void) {
  __asm volatile("cpsid i"); /* disable all interrupts, they get enabled at scheduler start */
  Platform_Init();
#if McuLib_CONFIG_SDK_USE_FREERTOS
  vTaskStartScheduler();
#endif
  for(;;) {} /* should not get here */
}
#endif /* PL_CONFIG_IS_ROBOT */
