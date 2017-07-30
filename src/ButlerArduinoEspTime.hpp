/*
 *******************************************************************************
 *
 * Purpose: Time functions implementation for ESP
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_ESP_TIME_H_
#define BUTLER_ARDUINO_ESP_TIME_H_

/* System Includes */
#include <Arduino.h>
#include <time.h>
/* Internal Includes */
#include "ButlerArduinoTime.hpp"

#ifndef BUTLER_ARDUINO_ESP_TIME_NTP_TIMEOUT_MS
	#define BUTLER_ARDUINO_ESP_TIME_NTP_TIMEOUT_MS						10000L
#endif

namespace Butler {
namespace Arduino {

namespace Time {

class EspClock: public Clock {
	public:
		virtual ~EspClock() {}

		virtual unsigned long millis() const {
			return ::millis();
		}

		virtual unsigned long rtc() const {
			return time(nullptr);
		}

		virtual void initRtc(const char* ntpServer) {
			configTime(0, 0, ntpServer);
			Timer timer(*this, BUTLER_ARDUINO_ESP_TIME_NTP_TIMEOUT_MS);
			while (!timer.expired() && !time(nullptr)) { yield(); }
		}
};

} // Time

}}

#endif // BUTLER_ARDUINO_ESP_TIME_H_
