/*
 *******************************************************************************
 *
 * Purpose: Hardware UART adaptor.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2015, 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_HWUART_H_
#define BUTLER_ARDUINO_HWUART_H_

/* System Includes */
#include <stddef.h>
#include <stdint.h>
#include <HardwareSerial.h>
/* Internal Includes */
#include "ButlerArduinoUart.hpp"


namespace Butler {
namespace Arduino {

struct HwUartConfig {
	uint32_t											speed;
};

class HwUart: public Uart {
public:
	HwUart(const HwUartConfig& config) {
		Serial.begin(config.speed);
		while (!Serial);
	}

	void setTimeout(unsigned long timeout) { Serial.setTimeout(timeout); }

	size_t readBytes(char *buffer, size_t length) { return Serial.readBytes(buffer, length); }

	size_t write(uint8_t c) { return Serial.write(c); }

	size_t print(const char c[]) { return Serial.print(c); }

	size_t println(const char c[]) { return Serial.println(c); }

	void flush(void) { Serial.flush(); }

	int available(void) { return Serial.available(); }
};

}}

#endif // BUTLER_ARDUINO_HWUART_H_

