/*
 *******************************************************************************
 *
 * Purpose: Print interface declaration.
 *    Usually used to print logs.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_PRINT_H_
#define BUTLER_ARDUINO_PRINT_H_

/* System Includes */
#include <stddef.h>
/* Internal Includes */


namespace Butler {
namespace Arduino {

class Print {
public:
	virtual ~Print() {}
	virtual size_t println(const char*) = 0;
};

template<class Print_t>
class PrintAdaptor: public Print {
public:
	PrintAdaptor(Print_t &print): mPrint(print) {}

	size_t println(const char* v) { return mPrint.println(v); }
private:
	Print_t												&mPrint;
};

}}

#endif // BUTLER_ARDUINO_PRINT_H_

