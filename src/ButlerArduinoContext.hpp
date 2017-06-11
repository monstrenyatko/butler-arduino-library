/*
 *******************************************************************************
 *
 * Purpose: Library Context implementation.
 *    Keeps configuration and shared resources.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_CONTEXT_H_
#define BUTLER_ARDUINO_CONTEXT_H_

/* System Includes */
#include <stddef.h>
/* Internal Includes */


namespace Butler {
namespace Arduino {

namespace Time {class Clock;}
class Print;
class Lpm;

struct Context {
	Time::Clock											*time = NULL;
	Print												*logger = NULL;
	Lpm													*lpm = NULL;
};

}}

#endif // BUTLER_ARDUINO_CONTEXT_H_

