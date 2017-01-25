/*
 *******************************************************************************
 *
 * Purpose: Low Power Mode implementation for AVR.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2014, 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_AVR_LPM_H_
#define BUTLER_ARDUINO_AVR_LPM_H_

/* System Includes */
#include <stdint.h>
/* Internal Includes */
#include "ButlerArduinoLpm.hpp"


namespace Butler {
namespace Arduino {

enum AvrLpmMode {
	LPM_MODE_IDLE,
	LPM_MODE_PWR_SAVE
};

struct AvrLpmConfig {
	uint8_t												pinLedAwake;
	AvrLpmMode											mode;
};

class AvrLpm: public Lpm {
public:
	AvrLpm(const AvrLpmConfig&);
	void idle(unsigned long ms);
private:
	AvrLpmConfig										mConfig;
	uint8_t												clock_0_TIMSK = 0;

	void resetClock();
	void stopClock();
	void startClockBig();
	void updateSysClockBig();
	void startClockSmall();
	void updateSysClockSmall();
	void stopSysClock();
	void restoreSysClock();
};

}}

#endif // BUTLER_ARDUINO_AVR_LPM_H_

