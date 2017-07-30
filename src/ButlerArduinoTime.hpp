/*
 *******************************************************************************
 *
 * Purpose: Time functions implementation
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_TIME_H_
#define BUTLER_ARDUINO_TIME_H_

/* System Includes */
/* Internal Includes */


namespace Butler {
namespace Arduino {

namespace Time {

inline unsigned long calcTimeElapsed(unsigned long currentTs, unsigned long startTs) {
	return currentTs - startTs;
}

inline bool isTimePassed(unsigned long currentTs, unsigned long startTs, unsigned long duration) {
	return calcTimeElapsed(currentTs, startTs) >= duration;
}

inline unsigned long calcTimeLeft(unsigned long currentTs, unsigned long startTs, unsigned long duration) {
	return isTimePassed(currentTs, startTs, duration) ? 0 : duration - calcTimeElapsed(currentTs, startTs);
}

class Clock {
	public:
		virtual ~Clock() {}

		/** Returns the current time in milliseconds. */
		virtual unsigned long millis() const = 0;

		/** Returns the UTC time in seconds since Jan 01, 1970, 00:00. */
		virtual unsigned long rtc() const = 0;

		/** Initialize the current time by NTP server. */
		virtual void initRtc(const char* ntpServer) = 0;
};

class Timer {
	public:
		Timer(const Clock& clock): mClock(clock) {
		}

		Timer(const Clock& clock, unsigned long durationMs): Timer(clock) {
			set(durationMs);
		}

		Timer(const Timer &timer, unsigned long minDurationMs, unsigned long maxDurationMs): Timer(timer.mClock) {
			unsigned long durationMs = (timer.leftMs() > maxDurationMs) ? maxDurationMs : timer.leftMs();
			set(durationMs > minDurationMs ? durationMs : minDurationMs);
		}

		void set(unsigned long durationMs) {
			mStartMs = mClock.millis();
			mDuration = durationMs;
		}

		void reset() {
			set(mDuration);
		}

		bool expired() const {
			return elapsedMs() >= mDuration;
		}

		unsigned long leftMs() const {
			return expired() ? 0 : mDuration - elapsedMs();
		}

		unsigned long elapsedMs() const {
			return mClock.millis() - mStartMs;
		}

	private:
		const Clock									&mClock;
		unsigned long								mStartMs = 0;
		unsigned long								mDuration = 0;
};

} // Time

}}

#endif // BUTLER_ARDUINO_TIME_H_

