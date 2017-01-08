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

#ifndef SWUART_H_
#define SWUART_H_

/* Internal Includes */
#include "Uart.h"
/* External Includes */
/* System Includes */

/* Forward declaration */
class SoftwareSerial;

struct SwUartConfig {
	uint32_t speed;
	uint8_t receivePin;
	uint8_t transmitPin;
};

class SwUart : public Uart {
public:
	SwUart(const SwUartConfig&);
	~SwUart();
	void setTimeout(unsigned long timeout);
	size_t readBytes(char *buffer, size_t length);
	size_t write(uint8_t c);
	size_t print(const char c[]);
	size_t println(const char c[]);
	void flush();
	int available(void);
private:
	SoftwareSerial*			mSerial;
};

#endif /* SWUART_H_ */
