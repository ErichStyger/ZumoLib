/**
 * \file
 * \brief Application for ESP32 specific parts.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_IS_ESP32
#include "appEsp.h"
#include "McuRTOS.h"
#include "McuLog.h"
#include "McuWiFi.h"
#include "McuNtpClient.h"
#include "McuMqttClient.h"
#include "McuUdpServer.h"
#include "debounce.h"
#include "buttons.h"
#if PL_CONFIG_USE_SENSIRION
  #include "sensirion.h"
#endif
#if PL_CONFIG_USE_MQTT_SENSOR
  #include "mqtt_sensor.h"
#endif
#if PL_CONFIG_USE_REMOTE_NORDIC
  #include "remoteNordic.h"
#endif

#if PL_CONFIG_USE_DEBOUNCE
void AppEsp_OnButtonEvent(Buttons_e button, McuDbnc_EventKinds event) { /* called from debouncing */
#if PL_CONFIG_USE_REMOTE_NORDIC
  RemoteNordic_ESP32OnButtonEvent(button, event);
#endif
}
#endif /* PL_CONFIG_USE_DEBOUNCE */

#if PL_CONFIG_USE_MQTT_CLIENT
static TaskHandle_t mqttTaskHandle = NULL;

static void AppEsp_MqttTaskResume(void) {
  if (mqttTaskHandle!=NULL) {
    vTaskResume(mqttTaskHandle);
  }
}

static void AppEsp_MqttTaskSuspend(void) {
  if (mqttTaskHandle!=NULL) {
    vTaskSuspend(mqttTaskHandle);
  }
}

static void AppEspMqttTask(void *pv) {
#if PL_CONFIG_USE_MQTT_SENSOR
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
#elif 0 && PL_CONFIG_USE_MQTT_GAME /* \TODO */
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

static void AppEsp_SuspendResumeNetworkServices(bool isSuspend) {
  if (isSuspend) {
  #if PL_CONFIG_USE_UDP_SERVER
    McuUdpServer_Suspend();
  #endif
  #if PL_CONFIG_USE_NTP_CLIENT
    McuNtpClient_TaskSuspend();
  #endif
  #if PL_CONFIG_USE_MQTT_CLIENT
    AppEsp_MqttTaskSuspend();
    McuMqttClient_Disconnect();
  #endif
  } else { /* resume */
  #if PL_CONFIG_USE_UDP_SERVER
    McuLog_info("resuming UDP server.");
    McuUdpServer_Resume();
  #endif
  #if PL_CONFIG_USE_NTP_CLIENT
    if (McuNtpClient_GetDefaultStart()) {
      McuLog_info("resuming NTP client.");
      McuNtpClient_TaskResume();
    }
  #endif
  #if PL_CONFIG_USE_MQTT_CLIENT
    if (McuMqttClient_Connect()!=ERR_OK) {
      McuLog_error("failed connecting to MQTT broker");
      McuMqttClient_Disconnect(); /* make sure it is disconnected */
    }
    AppEsp_MqttTaskResume();
  #endif
  }
}

static void AppEspTask(void *pv) {
  for(;;) {
#if PL_CONFIG_USE_BUTTONS && !PL_CONFIG_USE_BUTTONS_IRQ
    uint32_t buttons = Buttons_GetButtons();
    if (buttons!=0) { /* poll buttons */
      Debounce_StartDebounce(buttons);
    }
    vTaskDelay(pdMS_TO_TICKS(20));
#else
    vTaskDelay(pdMS_TO_TICKS(1000));
#endif
  }
}

void AppEsp_Init(void) {
#if PL_CONFIG_USE_WIFI
  McuWiFi_SetSuspendResumeCallback(AppEsp_SuspendResumeNetworkServices);
#endif
#if PL_CONFIG_USE_MQTT_CLIENT
  if (xTaskCreate(
      AppEspMqttTask,  /* pointer to the task */
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
  if (xTaskCreate(AppEspTask, "AppEsp", 1024/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+1, NULL) != pdPASS) {
    for(;;){} /* error */
  }
}

#endif /* PL_CONFIG_IS_ESP32 */