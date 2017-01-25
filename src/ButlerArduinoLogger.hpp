/*
 *******************************************************************************
 *
 * Purpose: Logging implementation
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2014, 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_LOGGER_H_
#define BUTLER_ARDUINO_LOGGER_H_

/* System Includes */
#include <stdio.h>
#include <stdarg.h>
/* Internal Includes */
#include "ButlerArduinoContext.hpp"
#include "ButlerArduinoPrint.hpp"


#ifndef LOG_ENABLED
#define LOG_ENABLED										0
#endif

#ifndef LOG_MEM_ENABLED
#define LOG_MEM_ENABLED									0
#endif

#if LOG_ENABLED
	#define LOG_PRINTFLN(ctx, fmt, ...)	Butler::Arduino::Logger::logln_P(ctx, PSTR(fmt), ##__VA_ARGS__)
#else
	#define LOG_PRINTFLN(ctx, fmt, ...)	((void)0)
#endif

#ifndef Arduino_h
	#define vsnprintf_P(fmt, ...) vsnprintf(fmt, ##__VA_ARGS__)
	#define PSTR(fmt) (fmt)
#endif

namespace Butler {
namespace Arduino {

namespace Logger {

const int LOG_SIZE_MAX									= 128;

inline void logln_P(Context& ctx, const char *fmt, ...) {
	if (ctx.logger) {
		char buf[LOG_SIZE_MAX];
		va_list ap;
		va_start(ap, fmt);
		vsnprintf_P(buf, LOG_SIZE_MAX, fmt, ap);
		va_end(ap);
		ctx.logger->println(buf);
#if LOG_MEM_ENABLED
		{
			uint8_t *heapptr, *stackptr;
			stackptr = (uint8_t*)malloc(4);										// use stackptr temporarily
			heapptr = stackptr;													// save value of heap pointer
			free(stackptr);														// free up the memory again
			stackptr =  (uint8_t *)(SP);										// save value of stack pointer
			snprintf_P(buf, LOG_SIZE_MAX, PSTR("SP:%u HP:%u"), stackptr, heapptr);
			ctx.logger->println(buf);
		}
#endif
	}
}

} // Logger

}}

#endif /* BUTLER_ARDUINO_LOGGER_H_ */

