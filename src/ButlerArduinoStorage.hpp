/*
 *******************************************************************************
 *
 * Purpose: Persistent storage implementation.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_STORAGE_H_
#define BUTLER_ARDUINO_STORAGE_H_

/* System Includes */
#include <stdint.h>
#include <EEPROM.h>
/* Internal Includes */


namespace Butler {
namespace Arduino {

class Storage {
public:
	static uint16_t size() {
		return EEPROM.length();
	}
};

}}

#endif // BUTLER_ARDUINO_STORAGE_H_

