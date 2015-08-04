/*
 *******************************************************************************
 *
 * Purpose: UART adaptor interface
 *
 *******************************************************************************
 * Copyright Monstrenyatko 2015.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef UART_H_
#define UART_H_

/* Internal Includes */
/* External Includes */
/* System Includes */
#include <stddef.h>
#include <stdint.h>

class Uart {
public:
	virtual ~Uart() {}
	virtual void setTimeout(unsigned long timeout) = 0;
	virtual size_t readBytes(char *buffer, size_t length) = 0;
	virtual size_t write(uint8_t c) = 0;
	virtual size_t println(const char c[]) = 0;
	virtual void flush() = 0;
	virtual int available(void) = 0;
};

#endif /* UART_H_ */
