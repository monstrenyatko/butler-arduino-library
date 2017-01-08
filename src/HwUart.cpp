/*
 *******************************************************************************
 *
 * Purpose: Hardware UART adaptor
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2015, 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

/* Internal Includes */
#include "HwUart.h"
/* External Includes */
#include <Arduino.h>
/* System Includes */


HwUart::HwUart(const HwUartConfig& config) {
	Serial.begin(config.speed);
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

size_t HwUart::print(const char c[]) {
	return Serial.print(c);
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
