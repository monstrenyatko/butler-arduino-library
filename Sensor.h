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
