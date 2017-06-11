/*
 *******************************************************************************
 *
 * Purpose: Persistent storage implementation for ESP.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_ESP_STORAGE_H_
#define BUTLER_ARDUINO_ESP_STORAGE_H_

/* System Includes */
#include <stdint.h>
#include <EEPROM.h>
#include <spi_flash.h>
/* Internal Includes */
#include "ButlerArduinoCrc.h"
#include "ButlerArduinoStorage.hpp"


namespace Butler {
namespace Arduino {

class EspStorage: public Storage {
public:
	uint32_t size() {
		return SPI_FLASH_SEC_SIZE;
	}

	uint32_t readSize() {
		Header header;
		readHeader(header);
		return header.length;
	}

	bool read(Buffer& buffer) {
		Header header;
		// Read HEADER
		readHeader(header);
		// Initialize to read DATA
		uint32_t dataIdx = sizeof(header);
		EEPROM.begin(sizeof(header) + header.length);
		// Read DATA
		for (uint32_t i = 0; i < header.length && i < buffer.size(); i++) {
			buffer.get()[i] = EEPROM.read(dataIdx++);
		}
		// Finalize
		EEPROM.end();
		// Verify
		return header.crc == Butler::Arduino::Crc::crc32(buffer.get(), buffer.size());
	}

	void write(const Buffer& buffer) {
		Header header;
		header.length = buffer.size();
		header.crc = Butler::Arduino::Crc::crc32(buffer.get(), buffer.size());
		// Write HEADER
		writeHeader(header);
		// Initialize to Write DATA
		uint32_t dataIdx = sizeof(header);
		EEPROM.begin(sizeof(header) + header.length);
		// Write DATA
		for (uint32_t i = 0; i < buffer.size(); i++) {
			EEPROM.write(dataIdx++, buffer.get()[i]);
		}
		// Finalize
		EEPROM.end();
	}

	void reset() {
		Header header;
		writeHeader(header);
	}

private:
	struct Header {
		uint32_t length = 0;
		uint32_t crc = 0;
	};

	void readHeader(Header& header) {
		// Initialize
		EEPROM.begin(sizeof(header));
		// Read
		for (uint32_t i = 0; i < sizeof(header); i++) {
			reinterpret_cast<uint8_t*>(&header)[i] = EEPROM.read(i);
		}
		// Finalize
		EEPROM.end();
	}

	void writeHeader(const Header& header) {
		// Initialize
		EEPROM.begin(sizeof(header));
		// Write
		for (uint32_t i = 0; i < sizeof(header); i++) {
			EEPROM.write(i, reinterpret_cast<const uint8_t*>(&header)[i]);
		}
		// Finalize
		EEPROM.end();
	}
};

}}

#endif // BUTLER_ARDUINO_STORAGE_H_

