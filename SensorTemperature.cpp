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
/* Internal Includes */
/* External Includes */
#include <DHT.h>


SensorTemperature::SensorTemperature(DHT& sensor)
: mSensor(sensor)
{}

SensorTemperature::~SensorTemperature() {
}

SensorValue SensorTemperature::getData() {
	return mSensor.readTemperature();
}

bool SensorTemperature::verify(SensorValue v) {
	return !isnan(v);
}
