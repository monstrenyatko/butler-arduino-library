/*
 *******************************************************************************
 *
 * Purpose: Array buffer implementation.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_ARRAY_BUFFER_H_
#define BUTLER_ARDUINO_ARRAY_BUFFER_H_

/* System Includes */
/* Internal Includes */
#include "ButlerArduinoBuffer.hpp"


namespace Butler {
namespace Arduino {

template<uint32_t BUFFER_SIZE = 100>
struct ArrayBuffer: public Buffer {
		uint8_t* get() const {return buf;}
		uint32_t size() const {return BUFFER_SIZE;}
	private:
		uint8_t											buf[BUFFER_SIZE];
};

}}

#endif // BUTLER_ARDUINO_ARRAY_BUFFER_H_

