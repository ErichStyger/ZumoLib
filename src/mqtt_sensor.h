/*
 * Copyright (c) 2025, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MQTT_SENSOR_H_
#define MQTT_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum topic_ID_e { /* IDs for topics we can subscribe to */
  Topic_ID_None,
  Topic_ID_Sensor_Enable,
  Topic_ID_Sensor_Temperature,
  Topic_ID_Sensor_Humidity,
} topic_ID_e;

bool MqttSensor_GetIsEnabled(void);

int MqttSensor_Publish_Enabled(bool isEnabled);

int MqttSensor_Publish_SensorValues(float temperature, float humidity);

void MqttSensor_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* MQTT_SENSOR_H_ */