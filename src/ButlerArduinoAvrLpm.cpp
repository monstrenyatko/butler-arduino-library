/*
 *******************************************************************************
 *
 * Purpose: Low Power Mode implementation for AVR.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2014, 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifdef __AVR__

/* System Includes */
#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
/* Internal Includes */
#include "ButlerArduinoAvrLpm.hpp"
#include "ButlerArduinoTime.hpp"


#define LPM_CLOCK_PRESCALER_1024	(_BV (CS20) | _BV (CS21) | _BV (CS22))
#define LPM_CLOCK_PRESCALER_128		(_BV (CS20) | _BV (CS22))
#define LPM_CLOCK_PRESCALER_64		(_BV (CS22))

#define WDT_PRESCALER_8S			( _BV (WDP3) | _BV (WDP0) )
#define WDT_PRESCALER_4S			( _BV (WDP3) )
#define WDT_PRESCALER_2S			( _BV (WDP2) | _BV (WDP1) | _BV (WDP0) )
#define WDT_PRESCALER_1S			( _BV (WDP2) | _BV (WDP1) )
#define WDT_PRESCALER_500MS			( _BV (WDP2) | _BV (WDP0) )
#define WDT_PRESCALER_250MS			( _BV (WDP2) )
#define WDT_PRESCALER_125MS			( _BV (WDP1) | _BV (WDP0) )
#define WDT_PRESCALER_64MS			( _BV (WDP1) )
#define WDT_PRESCALER_32MS			( _BV (WDP0) )
#define WDT_PRESCALER_16MS			( 0 )

#if F_CPU == 8000000
	#define LPM_CLOCK_SMALL_PRESCALER					64L
	#define LPM_CLOCK_SMALL_PRESCALER_TCCR2B			LPM_CLOCK_PRESCALER_64
	// 0.001/(1/(F_CPU)*SMALL_PRESCALER)
	#define LPM_CLOCK_SMALL_TICKS_MAX					(125-1) // zero is relative
	#define LPM_CLOCK_SMALL_INTERRUPT_MS_QTY			1
	#define LPM_CLOCK_BIG_PRESCALER						1024L
	#define LPM_CLOCK_BIG_PRESCALER_TCCR2B				LPM_CLOCK_PRESCALER_1024
	// 0.032/(1/(F_CPU)*BIG_PRESCALER)
	#define LPM_CLOCK_BIG_TICKS_MAX						(250-1) // zero is relative
	#define LPM_CLOCK_BIG_INTERRUPT_MS_QTY				32
#elif F_CPU == 16000000
	#define LPM_CLOCK_SMALL_PRESCALER					128L
	#define LPM_CLOCK_SMALL_PRESCALER_TCCR2B			LPM_CLOCK_PRESCALER_128
	// 0.001/(1/(F_CPU)*SMALL_PRESCALER)
	#define LPM_CLOCK_SMALL_TICKS_MAX					(125-1) // zero is relative
	#define LPM_CLOCK_SMALL_INTERRUPT_MS_QTY			1
	#define LPM_CLOCK_BIG_PRESCALER						1024L
	#define LPM_CLOCK_BIG_PRESCALER_TCCR2B				LPM_CLOCK_PRESCALER_1024
	#define LPM_CLOCK_BIG_INTERRUPT_MS_QTY				16
	// 0.016/(1/(F_CPU)*BIG_PRESCALER)
	#define LPM_CLOCK_BIG_TICKS_MAX						(250-1) // zero is relative
#else
	#error Unsupported CPU Frequency
#endif

#define LPM_CLOCK_BIG_TICKS_PER_1_MS	(0.001/(1./(F_CPU)*LPM_CLOCK_BIG_PRESCALER))
#define LPM_CLOCK_SMALL_TICKS_PER_1_MS	(0.001/(1./(F_CPU)*LPM_CLOCK_SMALL_PRESCALER))

volatile unsigned long									lpmClockInterruptsQty = 0;
extern volatile unsigned long							timer0_millis;

ISR (TIMER2_COMPA_vect)
{
	++lpmClockInterruptsQty;
}

ISR (WDT_vect)
{
	wdt_disable();
}

namespace Butler {
namespace Arduino {

AvrLpm::AvrLpm(const AvrLpmConfig& config)
: mConfig(config)
{
	pinMode(mConfig.pinLedAwake, OUTPUT);
	digitalWrite(mConfig.pinLedAwake, HIGH);
	wdt_disable();
	resetLpmClock();
	clock_0_TIMSK = TIMSK0;
}

void AvrLpm::idle(unsigned long ms) {
	unsigned long start = millis();
	while (ms > 0) {
		if (ms >= 16 && mConfig.mode == LPM_MODE_PWR_DOWN) {
			stopSysClock();
			// setup the WDT
			unsigned int prescaler = calcWdtPrescaler(ms);
			startWdt(prescaler);
			// go to low power
			digitalWrite(mConfig.pinLedAwake, LOW);
			ADCSRA &= ~(1 << ADEN);		//ADC OFF
			set_sleep_mode(SLEEP_MODE_PWR_DOWN);
			noInterrupts();
			sleep_enable();
			sleep_bod_disable();
			interrupts();
			sleep_cpu();
			sleep_disable();
			interrupts();
			ADCSRA |= (1 << ADEN);		//ADC ON
			power_all_enable();
			// awake from here
			digitalWrite(mConfig.pinLedAwake, HIGH);
			updateSysClock(calcWdtTimeout(prescaler));
			restoreSysClock();
		} else if (ms > 1) {
			bool clockSmall = (ms < LPM_CLOCK_BIG_INTERRUPT_MS_QTY);
			stopSysClock();
			if (clockSmall) {
				startLpmClock(LPM_CLOCK_SMALL_TICKS_MAX, LPM_CLOCK_SMALL_PRESCALER_TCCR2B);
			} else {
				startLpmClock(LPM_CLOCK_BIG_TICKS_MAX, LPM_CLOCK_BIG_PRESCALER_TCCR2B);
			}
			// go to low power
			digitalWrite(mConfig.pinLedAwake, LOW);
			{
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
				power_usart0_disable();
				power_timer0_disable();
				power_timer1_disable();
				// power_timer2_disable()	// used to keep time tracking
				power_twi_disable();
				sei();
				sleep_cpu();
				sleep_disable();
				sei();
				power_all_enable();
				// awake from here
				digitalWrite(mConfig.pinLedAwake, HIGH);
			}
			stopLpmClock();
			if(clockSmall) {
				updateSysClock(lpmClockInterruptsQty, LPM_CLOCK_SMALL_INTERRUPT_MS_QTY, LPM_CLOCK_SMALL_TICKS_PER_1_MS);
			} else {
				updateSysClock(lpmClockInterruptsQty, LPM_CLOCK_BIG_INTERRUPT_MS_QTY, LPM_CLOCK_BIG_TICKS_PER_1_MS);
			}
			restoreSysClock();
		} else {
			// do not sleep
		}
		// calculate time
		{
			unsigned long current = millis();
			unsigned long delta = Time::calcTimeElapsed(current, start);
			if (delta < ms) {
				ms -= delta;
				start = current;
			} else {
				ms = 0;
			}
		}
	}
}

void AvrLpm::startLpmClock(unsigned int ticksMax, unsigned int prescaler) {
	stopLpmClock();
	// set
	lpmClockInterruptsQty = 0;
	TCNT2 = 0;
	OCR2A = ticksMax;								// set TOP
	OCR2B = 0;
	GTCCR = _BV (PSRASY);							// reset prescaler
	TCCR2A = _BV (WGM21);							// CTC mode
	TCCR2B = prescaler;								// set prescaler
	// start
	TIMSK2 = _BV (OCIE2A);
}

void AvrLpm::stopLpmClock() {
	TCCR2B = 0; // No clock source (Timer/Counter stopped).
	TIMSK2 = 0; // disable interrupts
}

void AvrLpm::resetLpmClock() {
	// disable interrupts
	TIMSK2 = 0;
	// set clock source to asynchronous
	ASSR = _BV (AS2);
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

void AvrLpm::stopSysClock() {
	// save state
	clock_0_TIMSK = TIMSK0;
	// disable interrupts
	TIMSK0 = 0;
	// clean flags
	TIFR0 = 0;
}

void AvrLpm::restoreSysClock() {
	// restore
	TIMSK0 = clock_0_TIMSK;
}

void AvrLpm::updateSysClock(unsigned long lpmClockInterruptsQty, int lpmClockMsPerInterrupt, unsigned long lpmClockTicksPerMs) {
	unsigned long millis = lpmClockInterruptsQty * lpmClockMsPerInterrupt;
	// get milliseconds remainder after last interrupt
	double remainder = TCNT2/lpmClockTicksPerMs;
	millis += round(remainder); //add the remainder with rounding
	updateSysClock(millis);
}

void AvrLpm::updateSysClock(unsigned long millis) {
	noInterrupts();
	timer0_millis += millis;
	interrupts();
}

void AvrLpm::startWdt(unsigned int prescaler) {
	// disable interrupts
	noInterrupts();

	// disable WDT
	wdt_reset();
	wdt_disable();

	// clear the reset flag => we can not clear WDE if WDRF is set
	MCUSR &= ~(_BV (WDRF));

	// to change WDE or the prescaler, we need to
	// set WDCE (This will allow updates for 4 clock cycles).
	WDTCSR |= (_BV (WDCE) | _BV (WDE));

	// set new WDT prescaler value
	WDTCSR = prescaler;

	// enable and start WDT using interrupt only mode
	WDTCSR |= _BV (WDIE);

	// enable interrupts
	interrupts();
}

unsigned int AvrLpm::calcWdtPrescaler(unsigned long ms) {
	if (ms >= 8000) {
		return WDT_PRESCALER_8S;
	} else if (ms >= 4000) {
		return WDT_PRESCALER_4S;
	} else if (ms >= 2000) {
		return WDT_PRESCALER_2S;
	} else if (ms >= 1000) {
		return WDT_PRESCALER_1S;
	} else if (ms >= 500) {
		return WDT_PRESCALER_500MS;
	} else if (ms >= 250) {
		return WDT_PRESCALER_250MS;
	} else if (ms >= 125) {
		return WDT_PRESCALER_125MS;
	} else if (ms >= 64) {
		return WDT_PRESCALER_64MS;
	} else if (ms >= 32) {
		return WDT_PRESCALER_32MS;
	} else {
		return WDT_PRESCALER_16MS;
	}
}

unsigned long AvrLpm::calcWdtTimeout(unsigned int prescaler) {
	switch (prescaler) {
		case WDT_PRESCALER_8S:		return 8000;
		case WDT_PRESCALER_4S:		return 4000;
		case WDT_PRESCALER_2S:		return 2000;
		case WDT_PRESCALER_1S:		return 1000;
		case WDT_PRESCALER_500MS:	return 500;
		case WDT_PRESCALER_250MS:	return 250;
		case WDT_PRESCALER_125MS:	return 125;
		case WDT_PRESCALER_64MS:	return 64;
		case WDT_PRESCALER_32MS:	return 32;
		case WDT_PRESCALER_16MS:	return 16;
		default:					return 0;
	}
}

}}

#endif // __AVR__

