/*
 *******************************************************************************
 *
 * Purpose: Common interface for Sensors implementation
 *
 *******************************************************************************
 * Copyright Monstrenyatko 2014.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef SENSOR_H_
#define SENSOR_H_

/* Internal Includes */
/* External Includes */
/* System Includes */
#include <stdint.h>

class Sensor {
public:
	virtual ~Sensor() {}
	virtual int32_t getData() = 0;
};

#endif /* SENSOR_H_ */
