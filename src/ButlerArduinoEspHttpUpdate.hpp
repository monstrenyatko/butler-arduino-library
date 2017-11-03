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
#include <FS.h>
/* Internal Includes */
#include "ButlerArduinoStrings.hpp"
#include "ButlerArduinoLogger.hpp"
#include "ButlerArduinoContext.hpp"
#include "ButlerArduinoMd5.h"


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
		setupHttpClient(http, url, httpsFingerprint, authToken);
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

	HTTPUpdateResult updateFile(
		const String &name,
		const String &url,
		const String &httpsFingerprint = Strings::EMPTY,
		const String &authToken = Strings::EMPTY)
	{
		LOG_PRINTFLN(getContext(), "[update-file] start, name: %s", name.c_str());
		FSInfo fsInfo;
		String md5;
		// Cleanup
		SPIFFS.remove(Strings::FILE_NAME_TEMP_DOWNLOAD);
		// Get FS info
		if (!SPIFFS.info(fsInfo)) {
			LOG_PRINTFLN(getContext(), "[update-file] ERROR, Can't get FS info");
			return HTTP_UPDATE_FAILED;
		}
		// Verify name
		if (name.length() > fsInfo.maxPathLength) {
			LOG_PRINTFLN(getContext(), "[update-file] ERROR, Filename is too long");
			return HTTP_UPDATE_FAILED;
		}
		// Calculate file MD5 if available
		if (SPIFFS.exists(name)) {
			File f = SPIFFS.open(name, "r");
			md5 = Md5::md5(f);
			f.close();
		}
		// Open temporary file
		File f = SPIFFS.open(Strings::FILE_NAME_TEMP_DOWNLOAD, "w+");
		if (!f) {
			LOG_PRINTFLN(getContext(), "[update-file] ERROR, Can't open temporary file");
			return HTTP_UPDATE_FAILED;
		}
		//
		HTTPUpdateResult res = HTTP_UPDATE_FAILED;
		// Prepare HTTP request
		HTTPClient http;
		setupHttpClient(http, url, httpsFingerprint, authToken);
		if (md5.length()) {
			http.addHeader(Strings::HEADER_X_MD5, md5);
		}
		// Set headers retrieval list
		{
			const char* headers[] = {Strings::HEADER_X_MD5};
			http.collectHeaders(headers, 1);
		}
		// Send HTTP request
		int32_t httpCode = http.GET();
		if (httpCode > 0) {
			LOG_PRINTFLN(getContext(), "[update-file] Update status, code: %i", httpCode);
			switch(httpCode) {
				case HTTP_CODE_OK:
				{
					// Verify available space
					int32_t length = http.getSize();
					if (length > (fsInfo.totalBytes - fsInfo.usedBytes)) {
						LOG_PRINTFLN(getContext(), "[update-file] ERROR, Not enough space");
						break;
					}
					// Get MD5 header value
					if (http.hasHeader(Strings::HEADER_X_MD5)) {
						md5 = http.header(Strings::HEADER_X_MD5);
					} else {
						LOG_PRINTFLN(getContext(), "[update-file] ERROR, Missed MD5 header");
						break;
					}
					// Download file
					http.writeToStream(&f);
					http.end();
					// Verify length
					if (f.size() != length) {
						LOG_PRINTFLN(getContext(), "[update-file] ERROR, Incomplete, %lu != %li", f.size(), length);
						break;
					}
					// Verify MD5
					if (md5 != Md5::md5(f)) {
						LOG_PRINTFLN(getContext(), "[update-file] ERROR, MD5 check failed");
						break;
					}
					res = HTTP_UPDATE_OK;
				}
					break;
				case HTTP_CODE_NOT_MODIFIED:
					LOG_PRINTFLN(getContext(), "[update-file] No Updates");
					res = HTTP_UPDATE_NO_UPDATES;
					break;
				default:
					LOG_PRINTFLN(getContext(), "[update-file] ERROR, Update failed, error: %s",
							http.errorToString(httpCode).c_str()
					);
					break;
			}
		} else {
			LOG_PRINTFLN(getContext(), "[update-file] ERROR, Update status is UNK, error: %s",
					http.errorToString(httpCode).c_str()
			);
		}
		// Close temporary file
		f.close();
		// Finalize file name
		if (res == HTTP_UPDATE_OK) {
			// Remove current file if exists
			SPIFFS.remove(name);
			// Rename temporary file to requested name
			if (SPIFFS.rename(Strings::FILE_NAME_TEMP_DOWNLOAD, name)) {
				LOG_PRINTFLN(getContext(), "[update-file] updated");
			} else {
				LOG_PRINTFLN(getContext(), "[update-file] ERROR, Can't rename temporary file");
				res = HTTP_UPDATE_FAILED;
			}
		}
		return res;
	}

private:
	Context&										mCtx;

	Context& getContext() {
		return mCtx;
	}

	void setupHttpClient(
		HTTPClient &http,
		const String &url,
		const String &httpsFingerprint,
		const String &authToken)
	{
		bool isSecure = false;
		if (httpsFingerprint.length()) {
			http.begin(url, httpsFingerprint);
			isSecure = true;
		} else {
			LOG_PRINTFLN(getContext(), "[update] WARN, Update via HTTP");
			http.begin(url);
		}
		if (isSecure && authToken.length()) {
			http.addHeader(Strings::HEADER_AUTHORIZATION, String(Strings::TOKEN) + Strings::SPACE + authToken);
		}
	}
};

}}

#endif // BUTLER_ARDUINO_ESP_HTTP_UPDATE_H_
