/*
 *******************************************************************************
 *
 * Purpose: Network interface declaration.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2014, 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_NETWORK_H_
#define BUTLER_ARDUINO_NETWORK_H_

/* System Includes */
/* Internal Includes */


namespace Butler {
namespace Arduino {

class Network {
public:
	virtual ~Network() {}
	virtual int connect(const char* hostname, int port) = 0;
	virtual int read(unsigned char* buffer, int len, unsigned long timeoutMs) = 0;
	virtual int write(unsigned char* buffer, int len, unsigned long timeoutMs) = 0;
	virtual int disconnect() = 0;
};

}}

#endif // BUTLER_ARDUINO_NETWORK_H_

