#include "SensorTemperature.h"
#include "GoliathMqttSensor.h"
/* Internal Includes */
/* External Includes */
/* System Includes */


SensorTemperature::~SensorTemperature() {
}

int32_t SensorTemperature::getData() {
	return 16;
}
