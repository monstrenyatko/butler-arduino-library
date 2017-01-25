/*
 *******************************************************************************
 *
 * Purpose: UART interface declaration.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2015, 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_UART_H_
#define BUTLER_ARDUINO_UART_H_

/* System Includes */
#include <stddef.h>
#include <stdint.h>
/* Internal Includes */
#include "ButlerArduinoPrint.hpp"


namespace Butler {
namespace Arduino {

class Uart: public Print {
public:
	virtual ~Uart() {}
	virtual void setTimeout(unsigned long timeout) = 0;
	virtual size_t readBytes(char *buffer, size_t length) = 0;
	virtual size_t write(uint8_t c) = 0;
	virtual size_t print(const char c[]) = 0;
	virtual size_t println(const char c[]) = 0;
	virtual void flush() = 0;
	virtual int available(void) = 0;
};

}}

#endif // BUTLER_ARDUINO_UART_H_

