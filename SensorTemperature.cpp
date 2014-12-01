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

#include "SensorTemperature.h"
#include "ArduinoMqttNode.h"
/* Internal Includes */
/* External Includes */
/* System Includes */


SensorTemperature::~SensorTemperature() {
}

int32_t SensorTemperature::getData() {
	//TODO: add real implementation
	return 16;
}
