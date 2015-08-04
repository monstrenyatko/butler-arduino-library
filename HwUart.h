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

#ifndef HWUART_H_
#define HWUART_H_

/* Internal Includes */
#include "Uart.h"
/* External Includes */
/* System Includes */


struct HwUartConfig {
	uint32_t speed;
};

class HwUart : public Uart {
public:
	HwUart(const HwUartConfig&);
	void setTimeout(unsigned long timeout);
	size_t readBytes(char *buffer, size_t length);
	size_t write(uint8_t c);
	size_t println(const char c[]);
	void flush();
	int available(void);
private:
	const HwUartConfig		mConfig;
};

#endif /* HWUART_H_ */
