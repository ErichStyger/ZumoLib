/*
 * Copyright (c) 2025, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file
 * \brief MQTT sensor interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
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

/*!
 * \brief Returns whether MQTT sensor publishing is enabled.
 * \return true if enabled, false otherwise.
 */
bool MqttSensor_GetIsEnabled(void);

/*!
 * \brief Publishes the MQTT sensor enabled state.
 * \param isEnabled Enabled state to publish.
 * \return Error code from the publish operation.
 */
int MqttSensor_Publish_Enabled(bool isEnabled);

/*!
 * \brief Publishes the current sensor values.
 * \param temperature Temperature value in degree Celsius.
 * \param humidity Relative humidity in percent.
 * \return Error code from the publish operation.
 */
int MqttSensor_Publish_SensorValues(float temperature, float humidity);

/*!
 * \brief Initializes the MQTT sensor module.
 */
void MqttSensor_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* MQTT_SENSOR_H_ */