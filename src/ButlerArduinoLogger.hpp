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
#include <Arduino.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
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
	#define BUTLER_PSTR(s)								PSTR(s)
	#define BUTLER_PSTR_ENABLED							1
#elif defined(ESP8266)
	#define BUTLER_PSTR(s)								(s)
	#define BUTLER_PSTR_ENABLED							0
#else
	#define BUTLER_PSTR(s)								(s)
	#define BUTLER_PSTR_ENABLED							0
#endif

#if LOG_ENABLED
	#define LOG_PRINTFLN(ctx, fmt, ...)			Butler::Arduino::Logger::logln_I(ctx, BUTLER_PSTR(fmt), ##__VA_ARGS__)
	#define LOG_PRINTFLN_LONG(ctx, fmt, str)	Butler::Arduino::Logger::logln_I_long(ctx, BUTLER_PSTR(fmt), str)
#else
	#define LOG_PRINTFLN(ctx, fmt, ...)			((void)0)
	#define LOG_PRINTFLN_LONG(ctx, fmt, str)	((void)0)
#endif


namespace Butler {
namespace Arduino {

namespace LoggerPrivate {

inline void vsnprintf_I(char* str, int strSize, const char *fmt, va_list ap) {
#if BUTLER_PSTR_ENABLED
				::vsnprintf_P(str, strSize, fmt, ap);
#else
				::vsnprintf(str, strSize, fmt, ap);
#endif
}

inline void snprintf_I(char* str, int strSize, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
#if BUTLER_PSTR_ENABLED
				::snprintf_P(str, strSize, fmt, ap);
#else
				::snprintf(str, strSize, fmt, ap);
#endif
	va_end(ap);
}

inline size_t strlen_I(const char *str) {
#if BUTLER_PSTR_ENABLED
				return ::strlen_P(str);
#else
				return ::strlen(str);
#endif
}

} // LoggerPrivate

namespace Logger {

const int LOG_SIZE_MAX									= 128;

inline void logln_I(Context& ctx, const char *fmt, ...) {
	if (ctx.logger) {
		char buf[LOG_SIZE_MAX + 1];
		{
			va_list ap;
			va_start(ap, fmt);
			LoggerPrivate::vsnprintf_I(buf, LOG_SIZE_MAX, fmt, ap);
			va_end(ap);
			buf[LOG_SIZE_MAX] = '\0';
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
			LoggerPrivate::snprintf_I(buf, LOG_SIZE_MAX, BUTLER_PSTR("SP:%u HP:%u"), stackptr, heapptr);
			buf[LOG_SIZE_MAX] = '\0';
			ctx.logger->println(buf);
		}
	#endif // __AVR__
#endif
	}
}

inline void logln_I_long(Context& ctx, const char* fmt, const char* str) {
	const uint8_t fmtSize = LoggerPrivate::strlen_I(fmt);
	if (fmtSize >= LOG_SIZE_MAX) {
		logln_I(ctx, BUTLER_PSTR("Too long format"));
	} else {
		const uint8_t chunkMaxSize = LOG_SIZE_MAX - fmtSize;
		char chunkBuffer[chunkMaxSize + 1];
		const char* cur = str;
		uint32_t curSize;
		do {
			curSize = strlcpy(
				chunkBuffer,
				cur,
				sizeof(chunkBuffer)
			);
			logln_I(ctx, fmt, chunkBuffer);
			cur += chunkMaxSize;
		} while(curSize > chunkMaxSize);
	}
}

} // Logger

}}

#endif /* BUTLER_ARDUINO_LOGGER_H_ */

