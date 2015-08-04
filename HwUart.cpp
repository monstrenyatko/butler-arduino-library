/*
 *******************************************************************************
 *
 * Purpose: Hardware UART adaptor
 *
 *******************************************************************************
 * Copyright Monstrenyatko 2015.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

/* Internal Includes */
#include "HwUart.h"
#include "ArduinoMqttNode.h"
/* External Includes */
/* System Includes */


HwUart::HwUart(const HwUartConfig& config)
: mConfig(config)
{
	Serial.begin(mConfig.speed);
	while (!Serial);
}

void HwUart::setTimeout(unsigned long timeout) {
	Serial.setTimeout(timeout);
}

size_t HwUart::readBytes(char *buffer, size_t length) {
	return Serial.readBytes(buffer, length);
}

size_t HwUart::write(uint8_t c) {
	return Serial.write(c);
}

size_t HwUart::println(const char c[]) {
	return Serial.println(c);
}

void HwUart::flush() {
	Serial.flush();
}

int HwUart::available(void) {
	return Serial.available();
}
