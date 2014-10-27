/* Internal Includes */
/* External Includes */
/* System Includes */
#include <stdint.h>

#ifndef LPM_H_
#define LPM_H_

struct LpmConfig {
	uint8_t pinLedAwake;
	uint8_t pinToWakeUp;
};

/*
 * Low Power Manager
 */
class Lpm {
public:
	static void init(LpmConfig&);
	static void idle(uint32_t ms);
private:
	static LpmConfig mConfig;

	static void resetClock();
	static void stopClock();
	static void startClockBig();
	static void updateSysClockBig();
	static void startClockSmall();
	static void updateSysClockSmall();
};

#endif /* LPM_H_ */
