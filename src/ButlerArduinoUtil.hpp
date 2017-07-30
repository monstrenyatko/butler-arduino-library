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
/* Internal Includes */


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

} // Util

}}

#endif // BUTLER_ARDUINO_UTIL_H_
