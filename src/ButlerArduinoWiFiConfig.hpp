/*
 *******************************************************************************
 *
 * Purpose: WiFi configuration structure.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_WIFI_CONFIG_H_
#define BUTLER_ARDUINO_WIFI_CONFIG_H_

/* System Includes */
#include <WString.h>
/* Internal Includes */


namespace Butler {
namespace Arduino {

namespace Config {

struct WiFi {
	String											ssid;
	String											passphrase;

	bool isValid() const {
		return ssid.length();
	}

	bool isEqual(const WiFi &o) const {
		return ssid.equals(o.ssid)
			&& passphrase.equals(o.passphrase);
	}
};

} // Config

}}

#endif // BUTLER_ARDUINO_WIFI_CONFIG_H_
