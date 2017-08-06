/*
 *******************************************************************************
 *
 * Purpose: Authentication JSON configuration structure.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_AUTH_JSON_CONFIG_H_
#define BUTLER_ARDUINO_AUTH_JSON_CONFIG_H_

/* System Includes */
#include <WString.h>
/* Internal Includes */
#include "ButlerArduinoJsonConfig.hpp"
#include "ButlerArduinoAuthConfig.hpp"


namespace Butler {
namespace Arduino {

namespace Config {

struct AuthJsonConfig: public JsonConfigNode, public AuthConfig<> {

	bool decode(JsonObject &json) {
		bool updated = false;
		// FINGERPRINTS
		{
			JsonArray &o = json[Strings::FINGERPRINTS];
			if (JsonArray::invalid() != o) {
				uint8_t idx = 0;
				// Set new values
				for(JsonArray::iterator it = o.begin();
						idx < getMaxFingerprintsQty() && it != o.end();
						++it)
				{
					const char *value = *it;
					if (value) {
						if (!fingerprints[idx].equals(value)) {
							fingerprints[idx] = value;
							updated = true;
						}
						++idx;
					}
				}
				// Reset empty slots
				updated = resetFingerprints(idx) || updated;
			}
		}
		// TOKEN
		{
			const char* v = json[Strings::TOKEN];
			if (v && !token.equals(v)) {
				token = v;
				updated = true;
			}
		}
		return updated;
	}

	void encode(JsonObject &json, JsonBuffer &jsonBuffer) {
		// FINGERPRINTS
		if (getFingerprintsQty()) {
			JsonArray &o = jsonBuffer.createArray();
			for (uint8_t i = 0; i < getMaxFingerprintsQty(); ++i) {
				if (fingerprints[i].length()) {
					o.add(fingerprints[i].c_str());
				}
			}
			json[Strings::FINGERPRINTS] = o;
		}
		// TOKEN
		if (token.length()) {
			json[Strings::TOKEN] = token.c_str();
		}
	}

	bool isValid() const {
		return AuthConfig::isValid();
	}
};

} // Config

}}

#endif // BUTLER_ARDUINO_AUTH_JSON_CONFIG_H_
