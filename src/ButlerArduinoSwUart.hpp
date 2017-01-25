/*
 *******************************************************************************
 *
 * Purpose: Software UART adaptor.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2015, 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_SWUART_H_
#define BUTLER_ARDUINO_SWUART_H_

/* System Includes */
#include <stddef.h>
#include <stdint.h>
#include <SoftwareSerial.h>
/* Internal Includes */
#include "ButlerArduinoUart.hpp"


namespace Butler {
namespace Arduino {

struct SwUartConfig {
	uint32_t											speed;
	uint8_t												receivePin;
	uint8_t												transmitPin;
};

class SwUart: public Uart {
public:
	SwUart(const SwUartConfig& config) {
		mSerial = new SoftwareSerial(config.receivePin, config.transmitPin);
		mSerial->begin(config.speed);
	}

	~SwUart() { delete mSerial; }

	void setTimeout(unsigned long timeout) { mSerial->setTimeout(timeout); }

	size_t readBytes(char *buffer, size_t length) { return mSerial->readBytes(buffer, length); }

	size_t write(uint8_t c) { return mSerial->write(c); }

	size_t print(const char c[]) { return mSerial->print(c); }

	size_t println(const char c[]) { return mSerial->println(c); }

	void flush(void) { mSerial->flush(); }

	int available(void) { return mSerial->available(); }
private:
	SoftwareSerial										*mSerial;
};

}}

#endif // BUTLER_ARDUINO_SWUART_H_

