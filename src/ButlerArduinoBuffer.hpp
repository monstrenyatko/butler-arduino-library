/*
 *******************************************************************************
 *
 * Purpose: Buffer interface declaration.
 *    Usually used to print logs.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_BUFFER_H_
#define BUTLER_ARDUINO_BUFFER_H_

/* System Includes */
#include <stdint.h>
/* Internal Includes */


namespace Butler {
namespace Arduino {

template<typename TYPE_T>
struct BufferBase {
	virtual ~BufferBase() {}
	/** Gets pointer to array */
	virtual TYPE_T* get() = 0;
	virtual const TYPE_T* get() const = 0;
	/** Gets the array size */
	virtual uint32_t size() const = 0;
};

struct Buffer: public BufferBase<uint8_t> {
	virtual ~Buffer() {}
};

}}

#endif // BUTLER_ARDUINO_BUFFER_H_

