/*
 *******************************************************************************
 *
 * Purpose: Common interface for Networking implementation
 *
 *******************************************************************************
 * Copyright Monstrenyatko 2014.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef NETWORK_H_
#define NETWORK_H_

/* Internal Includes */
/* External Includes */
/* System Includes */
#include <stdint.h>

class Network {
public:
	virtual ~Network() {}
	virtual int connect(const char* hostname, int port) = 0;
	virtual int connect(uint32_t hostname, int port) = 0;
	virtual int read(unsigned char* buffer, int len, int timeoutMs) = 0;
	virtual int write(unsigned char* buffer, int len, int timeoutMs) = 0;
	virtual int disconnect() = 0;
};

#endif // NETWORK_H_
