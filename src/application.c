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

#if PL_CONFIG_IS_ROBOT
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
#endif /* #if PL_CONFIG_IS_ROBOT */

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
#if PL_CONFIG_IS_ESP32 && PL_CONFIG_USE_REMOTE_NORDIC
  RemoteNordic_ESP32OnButtonEvent(button, event);
#endif
#if PL_CONFIG_IS_ROBOT
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
#endif /* PL_CONFIG_IS_ROBOT */
}
#endif /* PL_CONFIG_USE_DEBOUNCE */

#if PL_CONFIG_IS_ESP32
static void AppTask(void *pv) {
  for(;;) {
#if PL_CONFIG_USE_BUTTONS && !PL_CONFIG_USE_BUTTONS_IRQ
    /*! \TODO if enabled WiFi, it triggers GPIO button interrupts? Doing polling instead */
    uint32_t buttons;

    buttons = Buttons_GetButtons();
    if (buttons!=0) { /* poll buttons */
      Debounce_StartDebounce(buttons);
    }
    vTaskDelay(pdMS_TO_TICKS(20));
#else
    vTaskDelay(pdMS_TO_TICKS(1000));
#endif
  }
}
#endif /* PL_CONFIG_IS_ESP32 */

#if PL_CONFIG_IS_ROBOT
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
#endif /* PL_CONFIG_IS_ROBOT */

#if PL_CONFIG_IS_ROBOT
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
#endif /* PL_CONFIG_IS_ROBOT */

#if !PL_CONFIG_USE_MQTT_CLIENT
void Application_MqttTaskResume(void) {
  /* dummy, do nothing if called by WiFi task */
}

void Application_MqttTaskSuspend(void) {
  /* dummy, do nothing if called by WiFi task */
}
#else /* PL_CONFIG_USE_MQTT_CLIENT */
static TaskHandle_t mqttTaskHandle = NULL;

void Application_MqttTaskResume(void) {
  if (mqttTaskHandle!=NULL) {
    vTaskResume(mqttTaskHandle);
  }
}

void Application_MqttTaskSuspend(void) {
  if (mqttTaskHandle!=NULL) {
    vTaskSuspend(mqttTaskHandle);
  }
}

static void MqttTask(void *pv) {
#if PL_CONFIG_USE_MQTT_SENSOR || PL_CONFIG_USE_MQTT_GAME
  bool firstTime = true;
  bool isEnabled = false;
#endif
for(;;) {
#if PL_CONFIG_USE_MQTT_SENSOR
    if (McuMqttClient_CanPublish()) {
      if (firstTime) {
        firstTime = false;
        isEnabled = MqttSensor_GetIsEnabled();
        MqttSensor_Publish_Enabled(isEnabled);
      }
      if (MqttSensor_GetIsEnabled()) {
        float t, h;
        h = Sensirion_GetHumidity();
        t = Sensirion_GetTemperature();
        if (MqttSensor_Publish_SensorValues(t, h)!=ERR_OK) {
          McuLog_error("failed publishing sensor values");
        }
      }
    }
#elif PL_CONFIG_USE_MQTT_GAME
    if (MqttClient_CanPublish()) {
      McuLog_info("query robot battery status");
      Game_QueryRobotBatteryVoltage(); /* periodically query robot battery voltage */
      if (firstTime) {
        firstTime = false;
        isEnabled = MqttGame_GetIsEnabled();
        MqttGame_Publish_Enabled(isEnabled);
      }
      if (MqttGame_GetIsEnabled()) {
        if (MqttGame_Publish_RobotStatus()!=ERR_OK) {
          McuLog_error("failed publishing robot status");
        }
        if (MqttGame_Publish_EspStatus()!=ERR_OK) {
          McuLog_error("failed publishing ESP status");
        }
      }
    }
#endif
    vTaskDelay(pdMS_TO_TICKS(5000));
  } /* for */
}
#endif /* PL_CONFIG_USE_MQTT_CLIENT */

void Application_Init(void) {
#if PL_CONFIG_USE_APP_TASK
  if (xTaskCreate(AppTask, "App", 1024/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) {
    for(;;){} /* error */
  }
#endif
#if PL_CONFIG_USE_MQTT_CLIENT
  if (xTaskCreate(
      MqttTask,  /* pointer to the task */
      "mqtt", /* task name for kernel awareness debugging */
      4*1024/sizeof(StackType_t), /* task stack size */
      (void*)NULL, /* optional task startup argument */
      tskIDLE_PRIORITY+2,  /* initial priority */
      &mqttTaskHandle /* optional task handle to create */
    ) != pdPASS)
  {
    McuLog_fatal("Failed creating MQTT task");
    for(;;){} /* error! probably out of memory */
  }
#endif /* PL_CONFIG_USE_MQTT_CLIENT */
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
