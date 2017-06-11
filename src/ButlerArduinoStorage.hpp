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
/* Internal Includes */
#include "ButlerArduinoBuffer.hpp"


namespace Butler {
namespace Arduino {

class Storage {
public:
	virtual ~Storage() {}
	virtual uint32_t size() = 0;
	virtual uint32_t readSize() = 0;
	virtual bool read(Buffer&) = 0;
	virtual void write(const Buffer&) = 0;
	virtual void reset() = 0;
};

}}

#endif // BUTLER_ARDUINO_STORAGE_H_

