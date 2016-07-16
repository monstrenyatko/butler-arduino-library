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
#include <DHT.h>
/* Internal Includes */
#include "Logger.h"
#include "Network.h"
#include "UartNetwork.h"
#include "SensorTemperature.h"
#include "SensorHumidity.h"
#include "Lpm.h"
#include "SwUart.h"
#include "HwUart.h"


////////// CONFIGURATION //////////
#define SENSOR_ID									"4A16-AEB6"
#define DHTTYPE										DHT11
#define PIN_DHT										2
#define PIN_SW_UART_RX								10
#define PIN_SW_UART_TX								11
#define PIN_LPM_NETWORK								9
#define PIN_LED_AWAKE								13
#define MQTT_HOST									"STUB"
#define MQTT_PORT									0
#define MQTT_MAX_PACKET_SIZE						128
#define MQTT_MAX_PAYLOAD_SIZE						96
#define MQTT_MAX_MESSAGE_HANDLERS					1
#define MQTT_COMMAND_TIMEOUT_MS						8000				// 8 sec
#define MQTT_KEEP_ALIVE_INTERVAL_SEC				5*60				// 5 min
#define MQTT_CLIENT_ID								SENSOR_ID
#define MQTT_SUBSCRIBE_QOS							MQTT::QOS1
#define MQTT_SUBSCRIBE_TOPIC_CFG					"butler/sensor/"SENSOR_ID"/config"
#define MQTT_PUBLISH_QOS							MQTT::QOS0
#define MQTT_PUBLISH_TOPIC							"butler/sensor/"SENSOR_ID"/data"
#define MQTT_LISTEN_TIME_MS							1000				// 1 sec
#define MQTT_PUBLISH_PERIOD_MS						3*60*1000L			// 3 min
#define MQTT_PUBLISH_PERIOD_MAX_MS					((MQTT_KEEP_ALIVE_INTERVAL_SEC - 5) * 1000L)
#define MQTT_CONNECT_RETRIES_QTY					5
#define MQTT_CONNECT_RETRIES_IDLE_PERIOD_MS			5000				// 5 sec
#define MQTT_DISCONNECTED_IDLE_PERIOD_MS			((MQTT_KEEP_ALIVE_INTERVAL_SEC + 5) * 1000L)
#define NETWORK_UART_IDLE_PERIOD_LONG_MS			16
#define NETWORK_UART_IDLE_PERIOD_SHORT_MS			2
#define NETWORK_HIBERNATE_DELAY_MS					2000
#define NETWORK_WAKE_UP_DELAY_MS					10000
#define LPM_MODE									LPM_MODE_IDLE
#define HW_UART_SPEED								57600
#define SW_UART_SPEED								57600


////////// OBJECTS //////////
Network* network = NULL;
MQTT::Client<Network, Countdown, MQTT_MAX_PACKET_SIZE, MQTT_MAX_MESSAGE_HANDLERS>* mqtt =
		NULL;
DHT* dht = NULL;
Sensor* sensorTemperature = NULL;
Sensor* sensorHumidity = NULL;
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
void networkHibernate(uint32_t);
void networkWakeUp(uint32_t);

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
		dht = new DHT(PIN_DHT, DHTTYPE);
		dht->begin();
		sensorTemperature = new SensorTemperature(*dht);
		sensorHumidity = new SensorHumidity(*dht);
	}

	////// INIT END //////
	LOG_PRINTFLN("#################################");
	LOG_PRINTFLN("###      Butler Sensor        ###");
	LOG_PRINTFLN("### ID          : %11s ###", SENSOR_ID);
	LOG_PRINTFLN("#################################");

	//// NETWORK START ////
	pinMode(PIN_LPM_NETWORK, OUTPUT);
	networkWakeUp(NETWORK_WAKE_UP_DELAY_MS);
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
					LOG_PRINTFLN("ERROR, Max retries qty has been reached => reset in %lu ms",
							MQTT_DISCONNECTED_IDLE_PERIOD_MS);
					networkHibernate(NETWORK_HIBERNATE_DELAY_MS);
					Lpm::idle(MQTT_DISCONNECTED_IDLE_PERIOD_MS - NETWORK_HIBERNATE_DELAY_MS);
					reset();
				}
			}
		}
	}
	{
		const uint8_t bufSize = MQTT_MAX_PAYLOAD_SIZE;
		char buf[bufSize];
		memset(buf, 0, bufSize);
		// build message payload
		{
			const int NUMBER_OF_SENSORS = 2;
			// See JSON_OBJECT_SIZE and JSON_ARRAY_SIZE to predict buffer size
			// lets make double the prediction
			const int BUFFER_SIZE = 2 * (JSON_OBJECT_SIZE(3)
					+ JSON_ARRAY_SIZE(NUMBER_OF_SENSORS)
					+ NUMBER_OF_SENSORS*JSON_OBJECT_SIZE(1));
			StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
			JsonArray& data = jsonBuffer.createArray();
			JsonObject& temp = jsonBuffer.createObject();
			{
				SensorValue v = sensorTemperature->getData();
				if (sensorTemperature->verify(v)) {
					temp["temp"] = v;
					data.add(temp);
				}
			}
			JsonObject& humid = jsonBuffer.createObject();
			{
				SensorValue v = sensorHumidity->getData();
				if (sensorHumidity->verify(v)) {
					humid["humid"] = v;
					data.add(humid);
				}
			}
			JsonObject& root = jsonBuffer.createObject();
			root["v"] = 1;
			root["id"] = SENSOR_ID;
			root["data"] = data;
			// write to buffer
			size_t size = root.printTo(buf, bufSize);
			if (size+1 >= bufSize) {
				LOG_PRINTFLN("WARN, Big payload: %d", size);
			}
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
	mqtt->yield(MQTT_LISTEN_TIME_MS);
	networkHibernate(NETWORK_HIBERNATE_DELAY_MS);
	Lpm::idle(publishPeriodMs
			- MQTT_LISTEN_TIME_MS
			- NETWORK_HIBERNATE_DELAY_MS
			- NETWORK_WAKE_UP_DELAY_MS
	);
	networkWakeUp(NETWORK_WAKE_UP_DELAY_MS);
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
	options.clientID.cstring = (char*)MQTT_CLIENT_ID;
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
	// TODO: change format to JSON
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

void networkHibernate(uint32_t delay) {
	Lpm::idle(delay);
	digitalWrite(PIN_LPM_NETWORK, HIGH);
}

void networkWakeUp(uint32_t delay) {
	digitalWrite(PIN_LPM_NETWORK, LOW);
	Lpm::idle(delay);
}
