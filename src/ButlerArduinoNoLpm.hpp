/*
 *******************************************************************************
 *
 * Purpose: No Low Power Mode implementation.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_NO_LPM_H_
#define BUTLER_ARDUINO_NO_LPM_H_

/* System Includes */
#include <stdint.h>
#include <Arduino.h>
/* Internal Includes */
#include "ButlerArduinoLpm.hpp"
#include "ButlerArduinoTime.hpp"
#include "ButlerArduinoContext.hpp"


namespace Butler {
namespace Arduino {

struct NoLpmConfig {
	uint8_t												pinLedAwake;
	Context												*gCtx;
};

class NoLpm: public Lpm {
public:
	NoLpm(const NoLpmConfig& config): mConfig(config) {
		pinMode(mConfig.pinLedAwake, OUTPUT);
		digitalWrite(mConfig.pinLedAwake, HIGH);
	}

	void idle(unsigned long ms) {
		Time::Timer timer(*(mConfig.gCtx->time), ms);
		digitalWrite(mConfig.pinLedAwake, LOW);
		while(!timer.expired());
		digitalWrite(mConfig.pinLedAwake, HIGH);
	}
private:
	NoLpmConfig										mConfig;
};

}}

#endif // BUTLER_ARDUINO_NO_LPM_H_

