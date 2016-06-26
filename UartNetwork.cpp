/*
 *******************************************************************************
 *
 * Purpose: Network over UART implementation
 *
 *******************************************************************************
 * Copyright Monstrenyatko 2014.
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

UartNetwork::UartNetwork(const UartNetworkConfig& config)
:mConfig(config)
{
}

int UartNetwork::connect(const char* hostname, int port) {
	return 0;
}

int UartNetwork::connect(uint32_t hostname, int port) {
	return 0;
}

int UartNetwork::read(unsigned char* buffer, int len, int timeoutMs) {
	mConfig.uart->setTimeout(timeoutMs);
	const int intervalMs = (timeoutMs < mConfig.readIdlePeriodLongMs)
			? mConfig.readIdlePeriodShortMs : mConfig.readIdlePeriodLongMs;
	int totalMs = 0;
	int readLen = 0;
	while(readLen < len && totalMs < timeoutMs) {
		int available = mConfig.uart->available();
		if (available) {
			int needToRead = len-readLen;
			int qty = mConfig.uart->readBytes((char*) buffer, needToRead);
			if (qty<0) {
				// error => return with current read length
				break;
			}
			buffer+=qty;
			readLen+=qty;
		} else {
			Lpm::idle(intervalMs);
			totalMs += intervalMs;
		}
	}
	return readLen;
}

int UartNetwork::write(unsigned char* buffer, int len, int timeoutMs) {
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
