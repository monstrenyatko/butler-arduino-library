/*
 *******************************************************************************
 *
 * Purpose: ESP implementation of the Captive Portal to request WiFi configuration.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_ESP_WIFI_CONFIG_CAPTIVE_PORTAL_H_
#define BUTLER_ARDUINO_ESP_WIFI_CONFIG_CAPTIVE_PORTAL_H_

/* System Includes */
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
/* Internal Includes */
#include "ButlerArduinoWiFiConfig.hpp"
#include "ButlerArduinoHtmlWifiConfig.h"
#include "ButlerArduinoStrings.hpp"

namespace Butler {
namespace Arduino {

namespace CaptivePortal {

typedef std::function<void(Config::WiFi&)> WiFiConfig_f;

class WiFiConfig {
public:
	WiFiConfig() {
		webServer = new ESP8266WebServer(80);
		dnsServer = new DNSServer();
	}

	~WiFiConfig() {
		delete dnsServer;
		delete webServer;
	}

	void start(const char* softApSsid,WiFiConfig_f handler) {
		//// NETWORK ////
		WiFi.persistent(false);
		WiFi.mode(WIFI_AP_STA);
		WiFi.softAP(softApSsid);

		//// WEB ////
		webServer->onNotFound([=]{
			// Redirect to Captive Portal if accessing some other IP
			if (!webServer->hostHeader().equals(webServer->client().localIP().toString())) {
				webServer->sendHeader("Location", String("http://") + webServer->client().localIP().toString(), true);
				webServer->send(302, "text/plain", "");
				webServer->client().stop();
			} else {
				webServer->send(404);
			}
		});
		webServer->on("/", [=]{
			// Show Captive Portal
			webServer->send(200, "text/html", FPSTR(Butler::Arduino::Html::WIFI_CONFIG_PAGE));
		});
		webServer->on("/wifiConfig", HTTP_POST, [=]{
			Config::WiFi v;
			v.ssid = webServer->arg(Butler::Arduino::Strings::SSID);
			v.passphrase = webServer->arg(Butler::Arduino::Strings::PASSPHRASE);
			webServer->send(204);
			webServer->client().stop();
			handler(v);
		});
		webServer->begin();

		//// DNS ////
		dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
		dnsServer->start(53, "*", WiFi.softAPIP());
	}

	void process() {
		dnsServer->processNextRequest();
		webServer->handleClient();
	}

private:
	ESP8266WebServer								*webServer;
	DNSServer										*dnsServer;
};

}

}}

#endif // BUTLER_ARDUINO_ESP_WIFI_CONFIG_CAPTIVE_PORTAL_H_
