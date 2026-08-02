/*
 * Copyright (c) 2025, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_MQTT_SENSOR
#if PL_CONFIG_USE_PICO_W
  #include "pico/cyw43_arch.h"
#endif
#include "lwip/apps/mqtt.h"
#include "mqtt_sensor_config.h"
#include "McuMqttClient.h"
#include "mqtt_sensor.h"
#include "McuRTOS.h"
#include "McuUtility.h"
#include "McuWatchdog.h"
#include "McuLog.h"
#include "cJSON.h"

static bool SensorIsEnabled = true;

bool MqttSensor_GetIsEnabled(void) {
  return SensorIsEnabled;
}

void MqttSensor_SetEnabledIsOn(bool isOn) {
  SensorIsEnabled = isOn;
}

void MqttSensor_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
  LWIP_UNUSED_ARG(data);

#if MQTT_CLIENT_CONFIG_EXTRA_LOGS
  const struct mqtt_connect_client_info_t *client_info = (const struct mqtt_connect_client_info_t*)arg;
  McuLog_trace("MQTT client \"%s\" data cb: len %d, flags %d", client_info->client_id, (int)len, (int)flags);
#endif
  if(flags & MQTT_DATA_FLAG_LAST) {
    topic_ID_e id = McuMqttClient_get_in_pub_ID();
    /* Last fragment of payload received (or whole part if payload fits receive buffer. See MQTT_VAR_HEADER_BUFFER_LEN)  */
    if (id == Topic_ID_Sensor_Temperature) {
      McuLog_trace("Temperature");
    } else if (id == Topic_ID_Sensor_Humidity) {
      McuLog_trace("Humidity");
    } else if (id == Topic_ID_Sensor_Enable) {
      McuLog_trace("Enable");
      McuMqttClient_IncomingSwitch(data, len, "sensor enable", MqttSensor_SetEnabledIsOn, false);
    } else {
      McuLog_trace("mqtt_incoming_data_cb, id: %d: Ignoring payload...", id);
    }
  } else {
    McuLog_trace("mqtt_incoming_data_cb: fragmented payload ...");
    /* Handle fragmented payload, store in buffer, write to file or whatever */
  }
}

void MqttSensor_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
  const struct mqtt_connect_client_info_t *client_info = (const struct mqtt_connect_client_info_t*)arg;
  McuLog_trace("incoming topic '%s'", topic);
  if (McuUtility_strcmp(topic, MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_HUMIDITY)==0) {
    McuMqttClient_set_in_pub_ID(Topic_ID_Sensor_Temperature);
  } else if (McuUtility_strcmp(topic, MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_HUMIDITY)==0) {
    McuMqttClient_set_in_pub_ID(Topic_ID_Sensor_Humidity);
  } else if (McuUtility_strcmp(topic, MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_ENABLE)==0) {
    McuMqttClient_set_in_pub_ID(Topic_ID_Sensor_Enable);
  } else { /* unknown */
    McuLog_trace("MQTT client \"%s\" incoming cb: topic %s, len %d", client_info->client_id, topic, (int)tot_len);
    McuMqttClient_set_in_pub_ID(Topic_ID_None);
  }
}

void MqttSensor_connection_cb(McuMqtt_client_handle client, void *arg, int /*mqtt_connection_status_t*/ status) {
  const struct mqtt_connect_client_info_t *client_info = (const struct mqtt_connect_client_info_t*)arg;

  if (status == MQTT_CONNECT_ACCEPTED) {
    /* subscribe to topics */
  	McuMqttClient_subscribeTopic(client, client_info, MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_ENABLE);
  }
}

int MqttSensor_Publish_Enabled(bool isEnabled) {
  uint8_t buf[64];

  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"{\"state\": ");
  if (isEnabled) {
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" \"ON\"");
  } else {
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" \"OFF\"");
  }
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"}");
  return McuMqttClient_PublishText(MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_ENABLE, (char*)buf);
}

int MqttSensor_Publish_SensorValues(float temperature, float humidity) {
  err_t res;
  uint8_t buf[64];

  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"{\"temperature\": ");
  McuUtility_strcatNumFloat(buf, sizeof(buf), temperature, 2);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)", \"unit\": \"°C\"}");
  res = McuMqttClient_PublishText(MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_TEMPERATURE, (char*)buf);
  if (res!=ERR_OK) {
    return res;
  }

  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"{\"humidity\": ");
  McuUtility_strcatNumFloat(buf, sizeof(buf), humidity, 2);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)", \"unit\": \"%\"}");
  return McuMqttClient_PublishText(MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_HUMIDITY, (char*)buf);
}

void MqttSensor_Init(void) {
  McuMqttClient_SetCallbacks(
    MqttSensor_connection_cb, 
    MqttSensor_incoming_data_cb, 
    MqttSensor_incoming_publish_cb);
}
#endif /* PL_CONFIG_USE_MQTT_SENSOR */
