/**
 * \file
 * \brief Application for robot specific parts.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_IS_ROBOT
#include "appRobot.h"
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

#if PL_CONFIG_USE_DEBOUNCE
void AppRobot_OnButtonEvent(Buttons_e button, McuDbnc_EventKinds event) { /* called from debouncing */
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

static void AppRobotTask(void *pv) {
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

void AppRobot_Main(void) {
  __asm volatile("cpsid i"); /* disable all interrupts, they get enabled at scheduler start */
  Platform_Init();
  vTaskStartScheduler();
  for(;;) {} /* should not get here */
}

void AppRobot_Init(void) {
  if (xTaskCreate(AppRobotTask, "AppRobot", 1024/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) {
    for(;;){} /* error */
  }
}

#endif /* PL_CONFIG_IS_ROBOT */