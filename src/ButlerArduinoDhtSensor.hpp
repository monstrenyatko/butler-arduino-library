/*
 *******************************************************************************
 *
 * Purpose: DHT Sensor implementation.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_DHT_SENSOR_H_
#define BUTLER_ARDUINO_DHT_SENSOR_H_

/* System Includes */
#include <math.h>
#include <DHT.h>
/* Internal Includes */
#include "ButlerArduinoSensor.h"


namespace Butler {
namespace Arduino {

class DhtSensor {
public:
	DhtSensor(DHT& sensor): mSensor(sensor) {mSensor.begin();}

	~DhtSensor() {}

	SensorValue getTemperature() { return mSensor.readTemperature(); }
	SensorValue getHumidity() { return mSensor.readHumidity(); }

	bool verify(SensorValue v) { return !isnan(v); }
private:
	DHT													&mSensor;
};

}}

#endif // BUTLER_ARDUINO_DHT_SENSOR_H_

