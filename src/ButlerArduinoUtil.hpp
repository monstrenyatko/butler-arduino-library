/*
 *******************************************************************************
 *
 * Purpose: Utility functions implementation
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_UTIL_H_
#define BUTLER_ARDUINO_UTIL_H_

/* System Includes */
#include <WString.h>
/* Internal Includes */
#include "ButlerArduinoStrings.hpp"


namespace Butler {
namespace Arduino {

#ifndef min
	template <class T> inline const T& min(const T& a, const T& b) {
		return b < a ? b : a;
	}
#endif

namespace Util {

String macAddressToHex(String mac) {
	int idx;
	while((idx = mac.indexOf(':')) >= 0) {
		mac.remove(idx, 1);
	}
	return mac;
}

void setModelKey(String &model, const String &key, const String &value) {
	model.replace(key, value);
}

String makeUrl(String model, const String &addr, const uint32_t port) {
	setModelKey(model, Strings::MODEL_KEY_ADDR, addr);
	setModelKey(model, Strings::MODEL_KEY_PORT, String(port));
	return model;
}

String makeTopic(String model, const String &nameSpace, const String &group, const String &id) {
	setModelKey(model, Strings::MODEL_KEY_NAMESPACE, nameSpace);
	setModelKey(model, Strings::MODEL_KEY_GROUP, group);
	setModelKey(model, Strings::MODEL_KEY_ID, id);
	return model;
}

} // Util

}}

#endif // BUTLER_ARDUINO_UTIL_H_
