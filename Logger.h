/*
 *******************************************************************************
 *
 * Purpose: Logging implementation
 *
 *******************************************************************************
 * Copyright Monstrenyatko 2014.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef LOGGER_H_
#define LOGGER_H_

/* Internal Includes */
/* External Includes */
/* System Includes */

#define LOG_ENABLED 0
#define LOG_SIZE_MAX 128

#if LOG_ENABLED
#define LOG_PRINTLN(str) Logger::println(str)
#define LOG_PRINTFLN(fmt, ...) Logger::printfln_P(PSTR(fmt), ##__VA_ARGS__)
#else
#define LOG_PRINTLN(str) ((void)0)
#define LOG_PRINTFLN(fmt, ...) ((void)0)
#endif

/* Forward declaration */
class Uart;

class Logger {
public:
	static void init(Uart&);
	static void println(const char*);
	static void printfln_P(const char *fmt, ...);
private:
	static Uart*		mOut;
};

#endif /* LOGGER_H_ */
