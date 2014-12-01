/*
 *******************************************************************************
 *
 * Purpose: Light Sensor implementation
 *
 *******************************************************************************
 * Copyright Monstrenyatko 2014.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#include "SensorLight.h"
#include "ArduinoMqttNode.h"
/* Internal Includes */
/* External Includes */
/* System Includes */


SensorLight::SensorLight(uint8_t analogPin)
:mPin(analogPin)
{}

SensorLight::~SensorLight() {
}

int32_t SensorLight::getData() {
	return map(analogRead(mPin), 0, 1024, 0, 100);
}

