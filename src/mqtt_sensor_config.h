/*
 * Copyright (c) 2025, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MQTT_SENSOR_CONFIG_H_
#define MQTT_SENSOR_CONFIG_H_

#ifndef MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_ENABLE
  #define MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_ENABLE           "home/roof/enable"
    /*!< topic name to enable/disable the sensor */
#endif

#ifndef MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_TEMPERATURE
  #define MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_TEMPERATURE      "home/roof/temperature"
    /*!< topic name for the sensor temperature */
#endif

#ifndef MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_HUMIDITY
  #define MQTT_SENSOR_CONFIG_TOPIC_NAME_SENSOR_HUMIDITY         "home/roof/humidity"
    /*!< topic name for the sensor humidity */
#endif

#endif /* MQTT_CLIENT_CONFIG_H_ */
