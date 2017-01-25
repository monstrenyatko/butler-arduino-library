/*
 *******************************************************************************
 *
 * Purpose: Low Power Mode interface declaration.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_LPM_H_
#define BUTLER_ARDUINO_LPM_H_

/* System Includes */
/* Internal Includes */


namespace Butler {
namespace Arduino {

class Lpm {
public:
	virtual ~Lpm() {}
	virtual void idle(unsigned long ms) = 0;
};

}}

#endif // BUTLER_ARDUINO_LPM_H_

