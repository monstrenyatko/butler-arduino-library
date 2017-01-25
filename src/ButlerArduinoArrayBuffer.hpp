/*
 *******************************************************************************
 *
 * Purpose: Array buffer implementation.
 *    Usually used to print logs.
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
#include <stddef.h>
/* Internal Includes */
#include "ButlerArduinoBuffer.hpp"


namespace Butler {
namespace Arduino {

template<int BUFFER_SIZE = 100>
struct ArrayBuffer: public Buffer {
		unsigned char* get() {return buf;}
		int size() const {return BUFFER_SIZE;}
	private:
		unsigned char								buf[BUFFER_SIZE];
};

}}

#endif // BUTLER_ARDUINO_ARRAY_BUFFER_H_

