/*
 *******************************************************************************
 *
 * Purpose: Software UART adaptor
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2015, 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

/* Internal Includes */
#include "SwUart.h"
/* External Includes */
/* System Includes */
#include <SoftwareSerial.h>

SwUart::SwUart(const SwUartConfig& config) {
	mSerial = new SoftwareSerial(config.receivePin, config.transmitPin);
	mSerial->begin(config.speed);
}

SwUart::~SwUart() {
	delete mSerial;
}

void SwUart::setTimeout(unsigned long timeout) {
	mSerial->setTimeout(timeout);
}

size_t SwUart::readBytes(char *buffer, size_t length) {
	return mSerial->readBytes(buffer, length);
}

size_t SwUart::write(uint8_t c) {
	return mSerial->write(c);
}

size_t SwUart::print(const char c[]) {
	return mSerial->print(c);
}

size_t SwUart::println(const char c[]) {
	return mSerial->println(c);
}

void SwUart::flush() {
	mSerial->flush();
}

int SwUart::available(void) {
	return mSerial->available();
}

