/*
 *******************************************************************************
 *
 * Purpose: Authentication configuration structure.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_AUTH_CONFIG_H_
#define BUTLER_ARDUINO_AUTH_CONFIG_H_

/* System Includes */
#include <WString.h>
/* Internal Includes */


namespace Butler {
namespace Arduino {

namespace Config {

template<uint8_t FINGERPRINTS_QTY = 2>
struct AuthConfig {
	bool											paired = false;
	String											fingerprints[FINGERPRINTS_QTY];
	String											token;

	AuthConfig() {
		for (uint8_t i = 0; i < getMaxFingerprintsQty(); ++i) {
			fingerprints[i] = String();
		}
	}

	void set(const AuthConfig &o) {
		paired = o.paired;
		for (uint8_t i = 0; i < getMaxFingerprintsQty(); ++i) {
			if (i < o.getMaxFingerprintsQty()) {
				fingerprints[i] = o.fingerprints[i];
			} else {
				fingerprints[i] = String();
			}
		}
		token = o.token;
	}

	bool isValid() const {
		return true;
	}

	bool isEqual(const AuthConfig &o) const {
		bool res = (paired == o.paired);
		res = res && (getMaxFingerprintsQty() == o.getMaxFingerprintsQty());
		for (uint8_t i = 0; res && i < getMaxFingerprintsQty(); ++i) {
			res = fingerprints[i].equals(o.fingerprints[i]);
		}
		res = res && (token.equals(o.token));
		return res;
	}

	uint8_t getMaxFingerprintsQty() const {
		return FINGERPRINTS_QTY;
	}

	uint8_t getFingerprintsQty() const {
		uint8_t res = 0;
		for (uint8_t i = 0; i < getMaxFingerprintsQty(); ++i) {
			if (fingerprints[i].length()) {
				++res;
			}
		}
		return res;
	}

	bool resetFingerprints(uint8_t startIdx = 0) {
		bool updated = false;
		for (uint8_t i = startIdx; i < getMaxFingerprintsQty(); ++i) {
			if (fingerprints[i].length()) {
				fingerprints[i] = String();
				updated = true;
			}
		}
		return updated;
	}
};

} // Config

}}

#endif // BUTLER_ARDUINO_AUTH_CONFIG_H_
