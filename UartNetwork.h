#ifndef UARTNETWORK_H_
#define UARTNETWORK_H_

/* Internal Includes */
#include "Network.h"
/* External Includes */
/* System Includes */

class UartNetwork: public Network {
public:
	static void init();
	int connect(const char* hostname, int port);
	int connect(uint32_t hostname, int port);
	int read(unsigned char* buffer, int len, int timeoutMs);
	int write(unsigned char* buffer, int len, int timeoutMs);
	int disconnect();
private:
};

#endif // UARTNETWORK_H_
