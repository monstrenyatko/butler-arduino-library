/*
 *******************************************************************************
 *
 * Purpose: Heap array buffer implementation.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_HEAP_ARRAY_BUFFER_H_
#define BUTLER_ARDUINO_HEAP_ARRAY_BUFFER_H_

/* System Includes */
/* Internal Includes */
#include "ButlerArduinoBuffer.hpp"


namespace Butler {
namespace Arduino {

struct HeapArrayBuffer: public Buffer {
		HeapArrayBuffer(): mSize(0), mBuf(NULL) {}
		HeapArrayBuffer(uint32_t size): mSize(size), mBuf(new uint8_t[mSize]) {}
		~HeapArrayBuffer() {delete[] mBuf;}
		uint8_t* get() const {return mBuf;}
		uint32_t size() const {return mSize;}
		void resize(uint32_t size) {delete[] mBuf; mSize = size; mBuf = new uint8_t[mSize];}
	private:
		uint32_t										mSize;
		uint8_t											*mBuf;
};

}}

#endif // BUTLER_ARDUINO_HEAP_ARRAY_BUFFER_H_

