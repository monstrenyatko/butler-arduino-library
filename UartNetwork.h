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

#ifndef UARTNETWORK_H_
#define UARTNETWORK_H_

/* Internal Includes */
#include "Network.h"
/* External Includes */
/* System Includes */

struct UartNetworkConfig {
	uint32_t speed;
	uint8_t readIdlePeriodLongMs;
	uint8_t readIdlePeriodShortMs;
};

class UartNetwork: public Network {
public:
	UartNetwork(const UartNetworkConfig&);
	int connect(const char* hostname, int port);
	int connect(uint32_t hostname, int port);
	int read(unsigned char* buffer, int len, int timeoutMs);
	int write(unsigned char* buffer, int len, int timeoutMs);
	int disconnect();
private:
	const UartNetworkConfig mConfig;
};

#endif // UARTNETWORK_H_
