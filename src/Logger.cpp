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

/* Internal Includes */
#include "Logger.h"
#include "Uart.h"
/* External Includes */
#include <Arduino.h>
/* System Includes */
#include <stdarg.h>


Uart* Logger::mOut = NULL;

void Logger::init(Uart& out) {
	mOut = &out;
}

void Logger::println(const char* v) {
	mOut->println(v);
}

void Logger::printfln_P(const char *fmt, ...) {
	char buf[LOG_SIZE_MAX];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf_P(buf, LOG_SIZE_MAX, fmt, ap);
	va_end(ap);
	mOut->println(buf);
}

