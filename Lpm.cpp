/*
 *******************************************************************************
 *
 * Purpose: Low Power Mode implementation
 *
 *******************************************************************************
 * Copyright Monstrenyatko 2014.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

/* Internal Includes */
#include "LPM.h"
#include "Logger.h"
/* External Includes */
#include <Arduino.h>
/* System Includes */
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>


#define LPM_CLOCK_PRESCALER_1024	(_BV (CS20) | _BV(CS21) | _BV (CS22))
#define LPM_CLOCK_PRESCALER_128		(_BV (CS20) | _BV (CS22))
#define LPM_CLOCK_PRESCALER_64		(_BV (CS22))

#if F_CPU == 8000000
	#define LPM_CLOCK_SMALL_PRESCALER					64
	#define LPM_CLOCK_SMALL_PRESCALER_TCCR2B			LPM_CLOCK_PRESCALER_64
	// 0.001/(1/(F_CPU)*SMALL_PRESCALER)
	#define LPM_CLOCK_SMALL_TICKS_MAX					(125-1) // zero is relative
	#define LPM_CLOCK_SMALL_INTERRUPT_MS_QTY			1
	#define LPM_CLOCK_BIG_PRESCALER						1024
	#define LPM_CLOCK_BIG_PRESCALER_TCCR2B				LPM_CLOCK_PRESCALER_1024
	// 0.032/(1/(F_CPU)*BIG_PRESCALER)
	#define LPM_CLOCK_BIG_TICKS_MAX						(250-1) // zero is relative
	#define LPM_CLOCK_BIG_INTERRUPT_MS_QTY				32
#elif F_CPU == 16000000
	#define LPM_CLOCK_SMALL_PRESCALER					128
	#define LPM_CLOCK_SMALL_PRESCALER_TCCR2B			LPM_CLOCK_PRESCALER_128
	// 0.001/(1/(F_CPU)*SMALL_PRESCALER)
	#define LPM_CLOCK_SMALL_TICKS_MAX					(125-1) // zero is relative
	#define LPM_CLOCK_SMALL_INTERRUPT_MS_QTY			1
	#define LPM_CLOCK_BIG_PRESCALER						1024
	#define LPM_CLOCK_BIG_PRESCALER_TCCR2B				LPM_CLOCK_PRESCALER_1024
	#define LPM_CLOCK_BIG_INTERRUPT_MS_QTY				16
	// 0.016/(1/(F_CPU)*BIG_PRESCALER)
	#define LPM_CLOCK_BIG_TICKS_MAX						(250-1) // zero is relative
#else
	#error Unsupported CPU Frequency
#endif

#define LPM_CLOCK_BIG_TICKS_PER_1_MS	(0.001/(1./(F_CPU)*LPM_CLOCK_BIG_PRESCALER))
#define LPM_CLOCK_SMALL_TICKS_PER_1_MS	(0.001/(1./(F_CPU)*LPM_CLOCK_SMALL_PRESCALER))

volatile unsigned long lpmClockInterruptsQty = 0;
uint8_t clock_0_TIMSK = 0;
LpmConfig Lpm::mConfig;

ISR (TIMER2_COMPA_vect)
{
	++lpmClockInterruptsQty;
}

void Lpm::init(const LpmConfig& config) {
	mConfig = config;
	pinMode(mConfig.pinLedAwake, OUTPUT);
	digitalWrite(mConfig.pinLedAwake, HIGH);
	resetClock();
	clock_0_TIMSK = TIMSK0;
}

void stopSysClock() {
	// save state
	clock_0_TIMSK = TIMSK0;
	// disable interrupts
	TIMSK0 = 0;
	// clean flags
	TIFR0 = 0;
}

void restoreSysClock() {
	// restore
	TIMSK0 = clock_0_TIMSK;
}

void Lpm::idle(uint32_t ms) {
	unsigned long start = millis();
	while (ms > 0) {
		if (ms>1) {
			bool clockSmall = (ms<LPM_CLOCK_BIG_INTERRUPT_MS_QTY);
			stopSysClock();
			if (clockSmall) {
				startClockSmall();
			} else {
				startClockBig();
			}
			// go to low power
			{
				digitalWrite(mConfig.pinLedAwake, LOW);
				switch(mConfig.mode) {
					case LPM_MODE_PWR_SAVE:
						set_sleep_mode(SLEEP_MODE_PWR_SAVE);
						break;
					case LPM_MODE_IDLE:
					default:
						set_sleep_mode(SLEEP_MODE_IDLE);
					break;
				}
				cli();
				sleep_enable();
				power_adc_disable();
				power_spi_disable();
				power_timer0_disable();
				power_timer1_disable();
				//power_timer2_disable() => need to keep time tracking
				power_twi_disable();
				sei();
				sleep_cpu();
				sleep_disable();
				sei();
				power_all_enable();
				// awake from here
				digitalWrite(mConfig.pinLedAwake, HIGH);
			}
			stopClock();
			if(clockSmall) {
				updateSysClockSmall();
			} else {
				updateSysClockBig();
			}
			restoreSysClock();
		}
		// calculate time
		{
			unsigned long current = millis();
			unsigned long delta = current - start;
			if (delta < ms) {
				ms -= delta;
				start = current;
			} else {
				ms = 0;
			}
		}
	}
}

void Lpm::startClockBig() {
	stopClock();
	// set
	lpmClockInterruptsQty = 0;
	TCNT2 = 0;
	OCR2A = LPM_CLOCK_BIG_TICKS_MAX;				// set TOP
	OCR2B = 0;
	GTCCR = _BV (PSRASY);							// reset prescaler
	TCCR2A = _BV(WGM21);							// CTC mode
	TCCR2B = LPM_CLOCK_BIG_PRESCALER_TCCR2B;		// set prescaler
	// start
	TIMSK2 = _BV(OCIE2A);
}

void Lpm::updateSysClockBig() {
	// 1 of .. == BIG_INTERRUPT_MS_QTY ms
	unsigned long tmp = lpmClockInterruptsQty*LPM_CLOCK_BIG_INTERRUPT_MS_QTY;
	// get ticks qty per 1ms
	double remainder = TCNT2/LPM_CLOCK_BIG_TICKS_PER_1_MS;
	tmp += round(remainder); //add the remainder with rounding
	extern volatile unsigned long timer0_millis;
	noInterrupts();
	timer0_millis += tmp;
	interrupts();
}

void Lpm::startClockSmall() {
	stopClock();
	// set
	lpmClockInterruptsQty = 0;
	TCNT2 = 0;
	OCR2A = LPM_CLOCK_SMALL_TICKS_MAX;				// set TOP
	OCR2B = 0;
	GTCCR = _BV (PSRASY);							// reset prescaler
	TCCR2A = _BV(WGM21);							// CTC mode
	TCCR2B = LPM_CLOCK_SMALL_PRESCALER_TCCR2B;		// set prescaler
	// start
	TIMSK2 = _BV(OCIE2A);
}

void Lpm::updateSysClockSmall() {
	// 1 of .. == SMALL_INTERRUPT_MS_QTY ms
	unsigned long tmp = lpmClockInterruptsQty*LPM_CLOCK_SMALL_INTERRUPT_MS_QTY;
	// get ticks qty per 1ms
	double remainder = TCNT2/LPM_CLOCK_SMALL_TICKS_PER_1_MS;
	tmp += round(remainder); //add the remainder with rounding
	extern volatile unsigned long timer0_millis;
	noInterrupts();
	timer0_millis += tmp;
	interrupts();
}

void Lpm::stopClock() {
	TCCR2B=0; // No clock source (Timer/Counter stopped).
	TIMSK2=0; // disable interrupts
}

void Lpm::resetClock() {
	// disable interrupts
	TIMSK2 = 0;
	// set clock source to asynchronous
	ASSR = _BV(AS2);
	// set all registers
	TCNT2 = 0;
	OCR2A = 0;
	OCR2B = 0;
	TCCR2A = 0;
	TCCR2B = 0;
	// wait a bit
	delay(1);
	// clean flags
	TIFR2 = 0;
}
