/*
 *******************************************************************************
 *
 * Purpose: ESP HTTP/S FW update implementation.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_ESP_HTTP_UPDATE_H_
#define BUTLER_ARDUINO_ESP_HTTP_UPDATE_H_

/* System Includes */
#include <Arduino.h>
#include <WString.h>
#include <ESP8266httpUpdate.h>
/* Internal Includes */
#include "ButlerArduinoStrings.hpp"
#include "ButlerArduinoLogger.hpp"
#include "ButlerArduinoContext.hpp"


namespace Butler {
namespace Arduino {

class EspHttpUpdate: private ESP8266HTTPUpdate {
public:
	EspHttpUpdate(Context& ctx): mCtx(ctx) { rebootOnUpdate(); }

	void rebootOnUpdate(bool reboot = true) {
		ESP8266HTTPUpdate::rebootOnUpdate(reboot);
	}

	HTTPUpdateResult update(
		const String &url,
		const String &httpsFingerprint = Strings::EMPTY,
		const String &authToken = Strings::EMPTY)
	{
		LOG_PRINTFLN(getContext(), "[update-fw] start");
		HTTPClient http;
		if (httpsFingerprint.length()) {
			http.begin(url, httpsFingerprint);
		} else {
			LOG_PRINTFLN(getContext(), "[update-fw] WARN, Update via HTTP");
			http.begin(url);
		}
		if (authToken.length()) {
			http.addHeader(Strings::HEADER_AUTHORIZATION, String(Strings::TOKEN) + Strings::SPACE + authToken);
		}
		HTTPUpdateResult res = handleUpdate(http, Strings::EMPTY, false);
		switch (res) {
			case HTTP_UPDATE_FAILED:
				LOG_PRINTFLN(getContext(), "[update-fw] ERROR, Update failed, error: %s",
						getLastErrorString().c_str()
				);
				break;
			case HTTP_UPDATE_NO_UPDATES:
				LOG_PRINTFLN(getContext(), "[update-fw] No Updates");
				break;
			case HTTP_UPDATE_OK:
				LOG_PRINTFLN(getContext(), "[update-fw] OK");
				break;
			default:
				LOG_PRINTFLN(getContext(), "[update-fw] ERROR, Update status is UNK, error: %s",
						getLastErrorString().c_str()
				);
				break;
		}
		return res;
	}
private:
	Context&										mCtx;

	Context& getContext() {
		return mCtx;
	}
};

}}

#endif // BUTLER_ARDUINO_ESP_HTTP_UPDATE_H_
