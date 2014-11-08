/*
 *******************************************************************************
 *
 * Purpose: Temperature Sensor implementation
 *
 *******************************************************************************
 * Copyright Monstrenyatko 2014.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef SENSOR_TEMPERATURE_H_
#define SENSOR_TEMPERATURE_H_

#include "Sensor.h"
/* Internal Includes */
/* External Includes */
/* System Includes */


class SensorTemperature: public Sensor {
public:
	~SensorTemperature();
	int32_t getData();
};

#endif /* SENSOR_TEMPERATURE_H_ */
