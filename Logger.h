#ifndef LOGGER_H_
#define LOGGER_H_

/* Internal Includes */
/* External Includes */
/* System Includes */

#define LOG_SIZE_MAX 128

#define LOG_PRINTLN(str) Logger::println(str)
#define LOG_PRINTFLN(fmt, ...) Logger::printfln_P(PSTR(fmt), ##__VA_ARGS__)

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
