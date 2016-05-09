/*
 *******************************************************************************
 *
 * Purpose: Humidity Sensor implementation
 *
 *******************************************************************************
 * Copyright Monstrenyatko 2014.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#include "butler-arduino-sensor.h"
#include "SensorHumidity.h"
/* Internal Includes */
/* External Includes */
#include <DHT.h>


SensorHumidity::SensorHumidity(DHT& sensor)
: mSensor(sensor)
{}

SensorHumidity::~SensorHumidity() {
}

SensorValue SensorHumidity::getData() {
	return mSensor.readHumidity();
}

bool SensorHumidity::verify(SensorValue v) {
	return !isnan(v);
}
