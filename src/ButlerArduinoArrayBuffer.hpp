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

template<typename TYPE_T, uint32_t BUFFER_SIZE>
struct ArrayBufferBase: public BufferBase<TYPE_T> {
		TYPE_T* get() {return buf;}
		const TYPE_T* get() const {return buf;}
		uint32_t size() const {return BUFFER_SIZE;}
	private:
		TYPE_T											buf[BUFFER_SIZE];
};

template<uint32_t BUFFER_SIZE = 100>
struct ArrayBuffer: public ArrayBufferBase<uint8_t, BUFFER_SIZE>, public Buffer {
	uint8_t* get() { return ArrayBufferBase<uint8_t, BUFFER_SIZE>::get(); }
	const uint8_t* get() const { return ArrayBufferBase<uint8_t, BUFFER_SIZE>::get(); }
	uint32_t size() const { return ArrayBufferBase<uint8_t, BUFFER_SIZE>::size(); }
};

template<uint32_t BUFFER_SIZE = 100>
struct CharArrayBuffer: public ArrayBufferBase<char, BUFFER_SIZE> {};
}}

#endif // BUTLER_ARDUINO_ARRAY_BUFFER_H_

