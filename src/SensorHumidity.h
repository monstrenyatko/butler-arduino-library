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

#ifndef SENSOR_HUMIDITY_H_
#define SENSOR_HUMIDITY_H_

#include "Sensor.h"
/* Internal Includes */
/* External Includes */
/* System Includes */

class DHT;

class SensorHumidity: public Sensor {
public:
	SensorHumidity(DHT&);
	~SensorHumidity();
	SensorValue getData();
	bool verify(SensorValue v);
private:
	DHT&	mSensor;
};

#endif /* SENSOR_HUMIDITY_H_ */
