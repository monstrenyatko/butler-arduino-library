#include "SensorLight.h"
#include "GoliathMqttSensor.h"
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

