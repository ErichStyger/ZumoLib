/*
 * Copyright (c) 2023-2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * \file
 * \brief Sensirion temperature and humidity sensor interface.
 * \author Erich Styger, erich.styger@hslu.ch
 * \license SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SENSOR_SENSIRION_H_
#define SENSOR_SENSIRION_H_

/*!
 * \brief Getter for the sensor temperature
 * \return Sensor temperature value in degree C
 */
float Sensirion_GetTemperature(void);

/*!
 * \brief Getter for the sensor humidity
 * \return Sensor humidity value as % RH
 */
float Sensirion_GetHumidity(void);

/*!
 * \brief Sensor module de-initialization
 */
void Sensirion_Deinit(void);

/*!
 * \brief Sensor module initialization
 */
void Sensirion_Init(void);

#endif /* SENSOR_SENSIRION_H_ */
