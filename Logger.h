#ifndef LOGGER_H_
#define LOGGER_H_

/* Internal Includes */
/* External Includes */
/* System Includes */

#define LOG_ENABLED 1
#define LOG_SIZE_MAX 128

#if LOG_ENABLED
#define LOG_PRINTLN(str) Logger::println(str)
#define LOG_PRINTFLN(fmt, ...) Logger::printfln_P(PSTR(fmt), ##__VA_ARGS__)
#else
#define LOG_PRINTLN(str) ((void)0)
#define LOG_PRINTFLN(fmt, ...) ((void)0)
#endif

class Print;

class Logger {
public:
	static void init(Print&);
	static void println(const char*);
	static void printfln_P(const char *fmt, ...);
private:
	static Print*		mOut;
};

#endif /* LOGGER_H_ */
