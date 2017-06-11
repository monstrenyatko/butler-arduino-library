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
	#define LOG_ENABLED									0
#endif

#ifndef LOG_MEM_ENABLED
	#define LOG_MEM_ENABLED								0
#endif

#ifdef __AVR__
	#define BUTLER_PSTR									PSTR(s)
	#define BUTLER_PSTR_ENABLED							1
#else
	#define BUTLER_PSTR(s)								s
	#define BUTLER_PSTR_ENABLED							0
#endif

#if LOG_ENABLED
	#define LOG_PRINTFLN(ctx, fmt, ...)	Butler::Arduino::Logger::logln_P(ctx, BUTLER_PSTR(fmt), ##__VA_ARGS__)
#else
	#define LOG_PRINTFLN(ctx, fmt, ...)	((void)0)
#endif


namespace Butler {
namespace Arduino {

namespace LoggerPrivate {

inline void vsnprintf_P(char* str, int strSize, const char *fmt, va_list ap) {
#if BUTLER_PSTR_ENABLED
				::vsnprintf_P(str, strSize, fmt, ap);
#else
				::vsnprintf(str, strSize, fmt, ap);
#endif
}

inline void snprintf_P(char* str, int strSize, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
#if BUTLER_PSTR_ENABLED
				::snprintf_P(str, strSize, fmt, ap);
#else
				::snprintf(str, strSize, fmt, ap);
#endif
	va_end(ap);
}

} // LoggerPrivate

namespace Logger {

const int LOG_SIZE_MAX									= 128;

inline void logln_P(Context& ctx, const char *fmt, ...) {
	if (ctx.logger) {
		char buf[LOG_SIZE_MAX];
		{
			va_list ap;
			va_start(ap, fmt);
			LoggerPrivate::vsnprintf_P(buf, LOG_SIZE_MAX, fmt, ap);
			va_end(ap);
			ctx.logger->println(buf);
		}
#if LOG_MEM_ENABLED
	#ifdef __AVR__
		{
			uint8_t *heapptr, *stackptr;
			stackptr = (uint8_t*)malloc(4);										// use stackptr temporarily
			heapptr = stackptr;													// save value of heap pointer
			free(stackptr);														// free up the memory again
			stackptr =  (uint8_t *)(SP);										// save value of stack pointer
			LoggerPrivate::snprintf_P(buf, LOG_SIZE_MAX, BUTLER_PSTR("SP:%u HP:%u"), stackptr, heapptr);
			ctx.logger->println(buf);
		}
	#endif // __AVR__
#endif
	}
}

} // Logger

}}

#endif /* BUTLER_ARDUINO_LOGGER_H_ */

