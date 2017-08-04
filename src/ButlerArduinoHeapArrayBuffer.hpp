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

template<typename TYPE_T>
struct HeapArrayBufferBase: public BufferBase<TYPE_T> {
		HeapArrayBufferBase(): mSize(0), mBuf(NULL) {}
		HeapArrayBufferBase(uint32_t size): mSize(size), mBuf(new TYPE_T[mSize]) {}
		~HeapArrayBufferBase() {delete[] mBuf;}
		TYPE_T* get() {return mBuf;}
		const TYPE_T* get() const {return mBuf;}
		uint32_t size() const {return mSize;}
		void resize(uint32_t size) {delete[] mBuf; mSize = size; mBuf = new TYPE_T[mSize];}
	private:
		uint32_t										mSize;
		TYPE_T											*mBuf;
};

struct HeapArrayBuffer: public HeapArrayBufferBase<uint8_t>, public Buffer {
	HeapArrayBuffer() {}
	HeapArrayBuffer(uint32_t size): HeapArrayBufferBase<uint8_t>(size) {}
	uint8_t* get() { return HeapArrayBufferBase<uint8_t>::get(); }
	const uint8_t* get() const { return HeapArrayBufferBase<uint8_t>::get(); }
	uint32_t size() const { return HeapArrayBufferBase<uint8_t>::size(); }
};

struct CharHeapArrayBuffer: public HeapArrayBufferBase<char> {
	CharHeapArrayBuffer() {}
	CharHeapArrayBuffer(uint32_t size): HeapArrayBufferBase<char>(size) {}
};

}}

#endif // BUTLER_ARDUINO_HEAP_ARRAY_BUFFER_H_

