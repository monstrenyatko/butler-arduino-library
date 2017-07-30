/*
 *******************************************************************************
 *
 * Purpose: Low Power Mode implementation for ESP.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_ESP_LPM_H_
#define BUTLER_ARDUINO_ESP_LPM_H_

/* System Includes */
#include <Arduino.h>
#include <stdint.h>
/* Internal Includes */
#include "ButlerArduinoLpm.hpp"
#include "ButlerArduinoCrc.h"


namespace Butler {
namespace Arduino {

class EspLpm: public Lpm {
public:
	void idle(unsigned long ms) {
		idle(ms, nullptr, 0);
	}

	void idle(uint32_t ms, uint32_t* data, uint32_t dataSize) {
		// Initialize
		LpmControl header;
		header.ctx.msCounter = ms;
		header.ctx.state = COUNT;
		if (data) {
			writeData(data, dataSize);
		}
		// Begin
		update(header, data, dataSize);
	}

	bool check(uint8_t resetPin, uint32_t* data = nullptr, uint32_t dataSize = 0) {
		if (LOW == digitalRead(resetPin)) {
			resetHeader();
			return false;
		}
		// Recover header and data
		LpmControl header;
		readHeader(header);
		if (data) {
			readData(data, dataSize);
		}
		// Check CRC
		{
			uint32_t crc = Crc::crc32Begin();
			crc = Crc::crc32Continue(crc, reinterpret_cast<uint8_t*>(&(header.ctx)), sizeof(header.ctx));
			if (data) {
				crc = Crc::crc32Continue(crc, reinterpret_cast<uint8_t*>(data), dataSize);
			}
			crc = Crc::crc32End(crc);
			if (header.crc != crc) {
				return false;
			}
		}
		// Continue
		update(header, data, dataSize);
		return true;
	}

	uint32_t getMaxDataSize() const {
		return 512 - sizeof(LpmControl);
	}
private:
	enum LpmState {
		COUNT,
		DONE
	};

	struct LpmControlCtx {
		LpmState										state = DONE;
		uint32_t										msCounter;
	} __attribute__((aligned(4)));

	struct LpmControl {
		uint32_t										crc;
		LpmControlCtx									ctx;
	} __attribute__((aligned(4)));

	const uint32_t										ONE_HOUR_MS = 1*60*60*1000;

	void update(LpmControl& header, uint32_t* data = nullptr, uint32_t dataSize = 0) {
		switch(header.ctx.state) {
			case COUNT:
			{
				// Update state
				RFMode rfMode = RF_DEFAULT;
				// Subtract current BOOT-time
				uint32_t bootTime = millis();
				header.ctx.msCounter -= (header.ctx.msCounter > bootTime) ? bootTime : header.ctx.msCounter;
				// Calculate next sleep time
				uint32_t sleepTimeMs = header.ctx.msCounter;
				if (sleepTimeMs > ONE_HOUR_MS) {
					sleepTimeMs = ONE_HOUR_MS;
					rfMode = WAKE_RF_DISABLED;
				} else {
					header.ctx.state = DONE;
				}
				// Update sleep time counter
				header.ctx.msCounter -= sleepTimeMs;
				// Update CRC
				{
					uint32_t crc = Crc::crc32Begin();
					crc = Crc::crc32Continue(crc, reinterpret_cast<uint8_t*>(&(header.ctx)), sizeof(header.ctx));
					if (data) {
						crc = Crc::crc32Continue(crc, reinterpret_cast<uint8_t*>(data), dataSize);
					}
					crc = Crc::crc32End(crc);
					header.crc = crc;
				}
				// Store state
				writeHeader(header);
				// Sleep
				ESP.deepSleep(sleepTimeMs * 1000L, rfMode);
			}
				break;
			case DONE:
				break;
			default:
				// Something wrong => wake-up
				resetHeader();
				break;
		}
	}

	void resetHeader() {
		LpmControl header;
		writeHeader(header);
	}

	void readHeader(LpmControl& header) {
		ESP.rtcUserMemoryRead(0, reinterpret_cast<uint32_t*>(&header), sizeof(LpmControl));
	}

	void writeHeader(const LpmControl& header) {
		ESP.rtcUserMemoryWrite(0, const_cast<uint32_t*>(reinterpret_cast<const uint32_t*>(&header)), sizeof(LpmControl));
	}

	void readData(uint32_t* data, uint32_t dataSize) {
		ESP.rtcUserMemoryWrite(sizeof(LpmControl), data, dataSize);
	}

	void writeData(const uint32_t* data, uint32_t dataSize) {
		ESP.rtcUserMemoryWrite(sizeof(LpmControl), const_cast<uint32_t*>(data), dataSize);
	}
};

}}

#endif // BUTLER_ARDUINO_ESP_LPM_H_

