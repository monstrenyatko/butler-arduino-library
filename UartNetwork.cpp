/* Internal Includes */
#include "UartNetwork.h"
#include "Lpm.h"
#include "GoliathMqttSensor.h"
/* External Includes */
/* System Includes */

UartNetwork::UartNetwork(const UartNetworkConfig& config)
:mConfig(config)
{
	Serial.begin(mConfig.speed);
	while (!Serial);
}

int UartNetwork::connect(const char* hostname, int port) {
	return 0;
}

int UartNetwork::connect(uint32_t hostname, int port) {
	return 0;
}

int UartNetwork::read(unsigned char* buffer, int len, int timeoutMs) {
	int intervalMs = mConfig.readIdlePeriodLongMs;
	int totalMs = 0;
	int rc = -1;
	Serial.setTimeout(timeoutMs);
	if (timeoutMs < intervalMs) {
		intervalMs = mConfig.readIdlePeriodShortMs;
	}
	while (Serial.available() < len && totalMs < timeoutMs) {
		Lpm::idle(intervalMs);
		totalMs += intervalMs;
	}
	if (Serial.available() >= len) {
		rc = Serial.readBytes((char*) buffer, len);
	}
	return rc;
}

int UartNetwork::write(unsigned char* buffer, int len, int timeoutMs) {
	Serial.setTimeout(timeoutMs);
	for (int i = 0; i < len; ++i) {
		Serial.write(buffer[i]);
	}
	Serial.flush();
	return len;
}

int UartNetwork::disconnect() {
	return 0;
}
