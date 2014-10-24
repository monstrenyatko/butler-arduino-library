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
