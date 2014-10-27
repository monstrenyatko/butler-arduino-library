/* Internal Includes */
#include "UartNetwork.h"
#include "Lpm.h"
#include "GoliathMqttSensor.h"
/* External Includes */
/* System Includes */

void UartNetwork::init() {
	Serial.begin(57600);
	while (!Serial)
		;
}

int UartNetwork::connect(const char* hostname, int port) {
	return 0;
}

int UartNetwork::connect(uint32_t hostname, int port) {
	return 0;
}

int UartNetwork::read(unsigned char* buffer, int len, int timeoutMs) {
	int intervalMs = 16;
	int totalMs = 0;
	int rc = -1;
	Serial.setTimeout(timeoutMs);
	if (timeoutMs < 30) {
		intervalMs = 2;
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
