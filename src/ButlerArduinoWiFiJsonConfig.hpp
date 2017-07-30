/*
 *******************************************************************************
 *
 * Purpose: WiFi Json configuration structure.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_WIFI_JSON_CONFIG_H_
#define BUTLER_ARDUINO_WIFI_JSON_CONFIG_H_

/* System Includes */
#include <WString.h>
/* Internal Includes */
#include "ButlerArduinoJsonConfig.hpp"
#include "ButlerArduinoWiFiConfig.hpp"


namespace Butler {
namespace Arduino {

namespace Config {

struct WiFiJsonConfig: public JsonConfigNode, public WiFiConfig {

	bool decode(JsonObject &json) {
		bool updated = false;
		// SSID
		{
			String v(json.containsKey(Strings::SSID)
				? json[Strings::SSID]
				: String(Strings::EMPTY)
			);
			if (!v.equals(ssid)) {
				ssid = v;
				updated = true;
			}
		}
		// PASSPHRASE
		{
			String v(json.containsKey(Strings::PASSPHRASE)
				? json[Strings::PASSPHRASE]
				: String(Strings::EMPTY)
			);
			if (!v.equals(passphrase)) {
				passphrase = v;
				updated = true;
			}
		}
		return updated;
	}

	void encode(JsonObject &json, JsonBuffer &jsonBuffer) {
		// SSID
		json[Strings::SSID] = ssid.c_str();
		// PASSPHRASE
		if (passphrase.length()) {
			json[Strings::PASSPHRASE] = passphrase.c_str();
		}
	}

	bool isValid() const {
		return WiFiConfig::isValid();
	}
};

} // Config

}}

#endif // BUTLER_ARDUINO_WIFI_JSON_CONFIG_H_
