/*
 *******************************************************************************
 *
 * Purpose: Network over UART implementation
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2014, 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

/* Internal Includes */
#include "UartNetwork.h"
#include "Lpm.h"
#include "Uart.h"
/* External Includes */
/* System Includes */
#include <Arduino.h>

UartNetwork::UartNetwork(const UartNetworkConfig& config)
:mConfig(config)
{
}

int UartNetwork::connect(const char* hostname, int port) {
	return 0;
}

int UartNetwork::read(unsigned char* buffer, int len, unsigned long timeoutMs) {
	unsigned long startMs = millis();
	mConfig.uart->setTimeout(timeoutMs);
	do {
		int qty = mConfig.uart->readBytes((char*) buffer, len);
		if (qty > 0) {
			return qty;
		}
	} while(millis() - startMs < timeoutMs);
	return 0;
}

int UartNetwork::write(unsigned char* buffer, int len, unsigned long timeoutMs) {
	mConfig.uart->setTimeout(timeoutMs);
	for (int i = 0; i < len; ++i) {
		mConfig.uart->write(buffer[i]);
	}
	mConfig.uart->flush();
	return len;
}

int UartNetwork::disconnect() {
	return 0;
}
