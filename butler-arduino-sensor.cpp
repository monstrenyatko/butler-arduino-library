/*
 *******************************************************************************
 *
 * Purpose: Arduino "Main" functions
 *
 *******************************************************************************
 * Copyright Monstrenyatko 2014,2015.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

/* External Includes */
#include <MemoryFree.h>
#include <Arduino.h>
#include <Countdown.h>
#include <MQTTClient.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
/* Internal Includes */
#include "Logger.h"
#include "Network.h"
#include "UartNetwork.h"
#include "SensorTemperature.h"
#include "SensorLight.h"
#include "Lpm.h"
#include "butler-arduino-sensor.h"
#include "SwUart.h"
#include "HwUart.h"


////////// CONFIGURATION //////////
#define PIN_SW_UART_RX								10
#define PIN_SW_UART_TX								11
#define PIN_LPM_WAKEUP								3
#define PIN_LED_AWAKE								13
#define PIN_SENSOR_LIGHT							A3
#define MQTT_HOST									"STUB"
#define MQTT_PORT									0
#define MQTT_MAX_PACKET_SIZE						80
#define MQTT_MAX_MESSAGE_HANDLERS					1
#define MQTT_COMMAND_TIMEOUT_MS						10000				// 10 sec
#define MQTT_KEEP_ALIVE_INTERVAL_SEC				30					// 30 sec
#define MQTT_CLIENT_ID								"MONSTRENYATKO_HOME_SENSOR_1234"
#define MQTT_SUBSCRIBE_QOS							MQTT::QOS1
#define MQTT_SUBSCRIBE_TOPIC_CFG					"monstrenyatko/home/cfg/sensor/1234"
#define MQTT_PUBLISH_QOS							MQTT::QOS1
#define MQTT_PUBLISH_TOPIC							"monstrenyatko/home/sensor/1234"
#define MQTT_PUBLISH_PERIOD_MS						15000				// 15 sec
#define MQTT_PUBLISH_PERIOD_MAX_MS					(1*60*60*1000)		// 1 hour
#define MQTT_CONNECT_RETRIES_QTY					5
#define MQTT_CONNECT_RETRIES_IDLE_PERIOD_MS			5000				// 5 sec
#define MQTT_DISCONNECTED_IDLE_PERIOD_MS			60000				// 1 min
#define NETWORK_UART_IDLE_PERIOD_LONG_MS			16
#define NETWORK_UART_IDLE_PERIOD_SHORT_MS			2
#define LPM_MODE									LPM_MODE_IDLE
#define HW_UART_SPEED								57600
#define SW_UART_SPEED								57600


////////// OBJECTS //////////
Network* network = NULL;
MQTT::Client<Network, Countdown, MQTT_MAX_PACKET_SIZE, MQTT_MAX_MESSAGE_HANDLERS>* mqtt =
		NULL;
Sensor* sensorTemperature = NULL;
Sensor* sensorLight = NULL;
unsigned long publishPeriodMs = MQTT_PUBLISH_PERIOD_MS;

SwUartConfig swUartConfig = {SW_UART_SPEED, PIN_SW_UART_RX, PIN_SW_UART_TX};
SwUart* swUart = NULL;

HwUartConfig hwUartConfig = {HW_UART_SPEED};
HwUart* hwUart = NULL;

//// DECLARATION ////
void(* reset) (void) = 0;//declare reset function at address 0
void check();
void connect();
void processMessageCfg(MQTT::MessageData& md);

////////// IMPLEMENTATION //////////
/**
 * Called once at startup
 */
void setup() {
	////// INIT //////

#if LOG_ENABLED
	/// HW UART ///
	{
		hwUart = new HwUart(hwUartConfig);
	}
#endif

	/// SF UART ///
	{
		swUart = new SwUart(swUartConfig);
	}

#if LOG_ENABLED
	//// LOG ////
	{
		Logger::init(*hwUart);
	}
#endif

	//// LPM ////
	{
		LpmConfig config;
		config.pinLedAwake = PIN_LED_AWAKE;
		config.mode = LPM_MODE;
		Lpm::init(config);
	}

	//// NETWORK ////
	{
		UartNetworkConfig config;
		config.uart = swUart;
		config.readIdlePeriodLongMs = NETWORK_UART_IDLE_PERIOD_LONG_MS;
		config.readIdlePeriodShortMs = NETWORK_UART_IDLE_PERIOD_SHORT_MS;
		network = new UartNetwork(config);
	}

	//// MQTT ////
	{
		mqtt = new MQTT::Client<Network, Countdown, MQTT_MAX_PACKET_SIZE,
		MQTT_MAX_MESSAGE_HANDLERS>(*network, MQTT_COMMAND_TIMEOUT_MS);
	}

	//// SENSOR ////
	{
		sensorTemperature = new SensorTemperature;
		sensorLight = new SensorLight(PIN_SENSOR_LIGHT);
	}

	////// INIT END //////
	LOG_PRINTFLN("#################################");
	LOG_PRINTFLN("###    MQTT Node is started   ###");
	LOG_PRINTFLN("#################################");
}

/**
 * Endless main loop
 */
void loop() {
	check();
	{
		int connectCounter = 0;
		while (!mqtt->isConnected()) {
			connect();
			if (!mqtt->isConnected()) {
				connectCounter++;
				if (connectCounter < MQTT_CONNECT_RETRIES_QTY ) {
					LOG_PRINTFLN("Reconnect in %u ms", MQTT_CONNECT_RETRIES_IDLE_PERIOD_MS);
					Lpm::idle(MQTT_CONNECT_RETRIES_IDLE_PERIOD_MS);
				} else {
					LOG_PRINTFLN("ERROR, Max retries qty has been reached => reset in %u ms",
							MQTT_DISCONNECTED_IDLE_PERIOD_MS);
					Lpm::idle(MQTT_DISCONNECTED_IDLE_PERIOD_MS);
					reset();
				}
			}
		}
	}
	{
		const uint8_t bufSize = 64;
		char buf[bufSize];
		memset(buf, 0, bufSize);
		// build message payload
		{
			const int NUMBER_OF_ELEMENTS = 3;
			// See JSON_OBJECT_SIZE and JSON_ARRAY_SIZE to predict buffer size
			// lets make double the prediction
			const int BUFFER_SIZE = JSON_OBJECT_SIZE(NUMBER_OF_ELEMENTS) * 2;
			StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
			JsonObject& root = jsonBuffer.createObject();
			root["v"] = 1;
			root["temperature"] = sensorTemperature->getData();
			root["light"] = sensorLight->getData();
			// write to buffer
			root.printTo(buf, bufSize);
			check();
		}
		// build message
		MQTT::Message message;
		message.qos = MQTT_PUBLISH_QOS;
		message.retained = false;
		message.dup = false;
		message.payload = (void*) buf;
		message.payloadlen = strlen(buf) + 1;
		int rc = mqtt->publish(MQTT_PUBLISH_TOPIC, message);
		if (rc != 0) {
			LOG_PRINTFLN("ERROR, MQTT publish, rc:%d", rc);
		}
	}
	LOG_PRINTFLN("Idle for %lu ms", publishPeriodMs);
	mqtt->yield(publishPeriodMs);
}

void check() {
	unsigned long time = millis();
	LOG_PRINTFLN("#################################");
	LOG_PRINTFLN("###      Periodic check       ###");
	LOG_PRINTFLN("### Memory Free :    %.5u B  ###", freeMemory());
	LOG_PRINTFLN("### Time        : %.8lu Ms ###", time);
	LOG_PRINTFLN("### Period      : %.8lu Ms ###", publishPeriodMs);
	LOG_PRINTFLN("#################################");
}

void connect() {
	int rc = network->connect(MQTT_HOST, MQTT_PORT);
	if (rc != 0) {
		LOG_PRINTFLN("ERROR, TCP connect, rc:%d", rc);
		return;
	}
	LOG_PRINTFLN("MQTT connect");
	MQTTPacket_connectData options = MQTTPacket_connectData_initializer;
	options.MQTTVersion = 3;
	options.clientID.cstring = MQTT_CLIENT_ID;
	options.cleansession = true;
	options.keepAliveInterval = MQTT_KEEP_ALIVE_INTERVAL_SEC;
	rc = mqtt->connect(options);
	if (rc != 0) {
		LOG_PRINTFLN("ERROR, MQTT connect, rc:%d", rc);
		return;
	}
	LOG_PRINTFLN("MQTT subscribe to %s", MQTT_SUBSCRIBE_TOPIC_CFG);
	rc = mqtt->subscribe(MQTT_SUBSCRIBE_TOPIC_CFG, MQTT_SUBSCRIBE_QOS, processMessageCfg);
	if (rc != 0) {
		LOG_PRINTFLN("ERROR, MQTT subscribe, rc:%d", rc);
		LOG_PRINTFLN("MQTT drop connection");
		mqtt->disconnect();
	}
}

void printMessage(const char* topic, const char* payload, MQTT::Message msg) {
	LOG_PRINTFLN(
			"Message arrived: qos %d, retained %d, dup %d, packetid %d, topic:[%s], payload:[%s]",
			msg.qos, msg.retained, msg.dup, msg.id, topic, payload);

}

void processMessageCfg(MQTT::MessageData& md) {
	const MQTT::Message& msg = md.message;
	const char* tokenSep = ",";
	const char blockSep = ':';
	// format is "name1:value,name2:value,nameN:value"
	char payload[msg.payloadlen + 1];
	memcpy(payload, msg.payload, msg.payloadlen);
	payload[msg.payloadlen] = '\0';
	printMessage(MQTT_SUBSCRIBE_TOPIC_CFG, payload, msg);
	char* token = strtok(payload, tokenSep);
	while (token != 0) {
		String name;
		String value;
		{
			String block(token);
			int sep = block.indexOf(blockSep);
			name = block.substring(0, sep);
			value = block.substring(sep + 1);
		}
		if (name.equals("period")) {
			char buf[value.length() + 1];
			value.toCharArray(buf, value.length() + 1);
			unsigned long tmp = value.toInt();
			if (tmp > 0) {
				publishPeriodMs = (tmp > MQTT_PUBLISH_PERIOD_MAX_MS) ?
				MQTT_PUBLISH_PERIOD_MAX_MS : tmp;
			}
		}
		// Find the next token
		token = strtok(0, tokenSep);
	}
}
