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

#ifndef UARTNETWORK_H_
#define UARTNETWORK_H_

/* Internal Includes */
#include "Network.h"
/* External Includes */
/* System Includes */

/* Forward declaration */
class Uart;

struct UartNetworkConfig {
	Uart*	uart;
};

class UartNetwork: public Network {
public:
	UartNetwork(const UartNetworkConfig&);
	int connect(const char* hostname, int port);
	int read(unsigned char* buffer, int len, unsigned long timeoutMs);
	int write(unsigned char* buffer, int len, unsigned long timeoutMs);
	int disconnect();
private:
	const UartNetworkConfig mConfig;
};

#endif // UARTNETWORK_H_
