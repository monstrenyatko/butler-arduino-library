/*
 *******************************************************************************
 *
 * Purpose: Network over UART implementation.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2014, 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_UARTNETWORK_H_
#define BUTLER_ARDUINO_UARTNETWORK_H_

/* System Includes */
/* Internal Includes */
#include "ButlerArduinoNetwork.hpp"
#include "ButlerArduinoUart.hpp"


namespace Butler {
namespace Arduino {

class UartNetwork: public Network {
public:
	UartNetwork(Uart& uart): mUart(uart) {}

	~UartNetwork() {}

	int connect(const char* hostname, int port) { return 0; }

	int read(unsigned char* buffer, int len, unsigned long timeoutMs) {
		mUart.setTimeout(timeoutMs);
		return mUart.readBytes((char*) buffer, len);
	}

	int write(unsigned char* buffer, int len, unsigned long timeoutMs) {
		mUart.setTimeout(timeoutMs);
		for (int i = 0; i < len; ++i) {
			mUart.write(buffer[i]);
		}
		mUart.flush();
		return len;
	}

	int disconnect() { return 0; }
private:
	Uart												&mUart;
};

}}

#endif // BUTLER_ARDUINO_UARTNETWORK_H_

