/*
 *******************************************************************************
 *
 * Purpose: Arduino "Main" functions
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2014, 2015, 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

/* External Includes */
#include <MemoryFree.h>
#include <Arduino.h>
#include <MqttClient.h>
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
#define SENSOR_ID									"TEST_SENSOR_ID"
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
#define MQTT_COMMAND_TIMEOUT_MS						(8*1000L)
#define MQTT_KEEP_ALIVE_INTERVAL_SEC				(MQTT_PUBLISH_PERIOD_MS/1000L*3)
#define MQTT_CLIENT_ID								SENSOR_ID
#define MQTT_SUBSCRIBE_QOS							MqttClient::QOS1
#define MQTT_SUBSCRIBE_TOPIC_CFG					"test/sensor/" SENSOR_ID "/config"
#define MQTT_PUBLISH_QOS							MqttClient::QOS0
#define MQTT_PUBLISH_TOPIC							"test/sensor/" SENSOR_ID "/data"
#define MQTT_LISTEN_TIME_MS							(5*1000L)
#define MQTT_PUBLISH_PERIOD_MS						(1*60*1000L)
#define MQTT_CONNECT_RETRIES_QTY					5
#define MQTT_DISCONNECTED_IDLE_PERIOD_MS			(2*60*1000L)
#define NETWORK_HIBERNATE_DELAY_MS					100L
#define NETWORK_WAKE_UP_DELAY_MS					100L
#define LPM_MODE									LPM_MODE_IDLE
#define HW_UART_SPEED								57600L
#define SW_UART_SPEED								9600L

////////// OBJECTS //////////
class Time: public MqttClient::Time {
public:
	unsigned long millis() const {
		return ::millis();
	}
} time;

Network *network									= NULL;
MqttClient *mqtt									= NULL;
Sensor* sensorTemperature							= NULL;
Sensor* sensorHumidity								= NULL;
unsigned long publishPeriodMs						= MQTT_PUBLISH_PERIOD_MS;
unsigned long publishTs								= 0;
int connectCounter									= 0;
bool firstPublish									= false;

//// DECLARATION ////
void(* reset) (void) = 0;//declare reset function at address 0
void check();
void connect();
void buildMessagePayload(char* buffer, int size);
void processMessageConfig(MqttClient::MessageData& md);
void networkHibernate(unsigned long);
void networkWakeUp(unsigned long);

////////// IMPLEMENTATION //////////
unsigned long timeElapsed(unsigned long startTs) {
	return time.millis() - startTs;
}

bool isTimePassed(unsigned long startTs, unsigned long duration) {
	return timeElapsed(startTs) >= duration;
}

unsigned long timeLeft(unsigned long startTs, unsigned long duration) {
	return isTimePassed(startTs, duration) ? 0 : duration - timeElapsed(startTs);
}

/** Called once at startup */
void setup() {
	////// INIT //////

	/// HW UART ///
	Uart *hwUart = new HwUart({HW_UART_SPEED});

	/// SF UART ///
	Uart *swUart = new SwUart({SW_UART_SPEED, PIN_SW_UART_RX, PIN_SW_UART_TX});

	//// LOG ////
	{
		Logger::init(*hwUart);
	}

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
		network = new UartNetwork(config);
	}

	//// MQTT ////
	{
		MqttClient::Logger *mqttLogger = new MqttClient::LoggerImpl<Uart>(*hwUart);
		MqttClient::Network * mqttNetwork = new MqttClient::NetworkImpl<Network>(*network, time);
		MqttClient::Buffer *mqttSendBuffer = new MqttClient::ArrayBuffer<MQTT_MAX_PACKET_SIZE>();
		MqttClient::Buffer *mqttRecvBuffer = new MqttClient::ArrayBuffer<MQTT_MAX_PACKET_SIZE>();
		MqttClient::MessageHandlers *mqttMessageHandlers = new MqttClient::MessageHandlersImpl<MQTT_MAX_MESSAGE_HANDLERS>();
		MqttClient::Options options;
		options.commandTimeoutMs = MQTT_COMMAND_TIMEOUT_MS;
		mqtt = new MqttClient (
			options, *mqttLogger, time, *mqttNetwork, *mqttSendBuffer,
			*mqttRecvBuffer, *mqttMessageHandlers
		);
	}

	//// SENSOR ////
	{
		DHT* dht = new DHT(PIN_DHT, DHTTYPE);
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
	if (!mqtt->isConnected()) {
		// Not connected
		// Clean buffers and give some time for network
		mqtt->yield();
		// Connect
		connect();
		if (!mqtt->isConnected()) {
			if (++connectCounter > MQTT_CONNECT_RETRIES_QTY ) {
				LOG_PRINTFLN("ERROR, Max retries qty has been reached => reset");
				reset();
				// Execution must stop at this point
			}
			networkHibernate(NETWORK_HIBERNATE_DELAY_MS);
			LOG_PRINTFLN("Reconnect in %lu ms", MQTT_DISCONNECTED_IDLE_PERIOD_MS);
			Lpm::idle(MQTT_DISCONNECTED_IDLE_PERIOD_MS);
			networkWakeUp(NETWORK_WAKE_UP_DELAY_MS);
		} else {
			// Do the publish immediately after reconnect
			firstPublish = true;
			// Listen for configuration
			LOG_PRINTFLN("Waiting the configuration for %lu ms", MQTT_LISTEN_TIME_MS);
			mqtt->yield(MQTT_LISTEN_TIME_MS);
		}
	} else {
		// Connected
		// Check if time to Publish
		if (firstPublish || isTimePassed(publishTs, publishPeriodMs)) {
			const int bufferSize = MQTT_MAX_PAYLOAD_SIZE;
			char buffer[bufferSize];
			memset(buffer, 0, bufferSize);
			// Build message payload
			buildMessagePayload(buffer, bufferSize);
			// Build message
			MqttClient::Message message;
			message.qos = MQTT_PUBLISH_QOS;
			message.retained = false;
			message.dup = false;
			message.payload = (void*) buffer;
			message.payloadLen = strlen(buffer) + 1;
			// Publish
			MqttClient::Error::type rc = mqtt->publish(MQTT_PUBLISH_TOPIC, message);
			if (rc != MqttClient::Error::SUCCESS) {
				LOG_PRINTFLN("ERROR, Publish, rc:%i", rc);
			} else {
				publishTs = time.millis();
				firstPublish = false;
			}
		}
		// Idle until next event
		LOG_PRINTFLN("Idle for %lu ms", min(timeLeft(publishTs, publishPeriodMs), mqtt->getIdleInterval()));
		networkHibernate(NETWORK_HIBERNATE_DELAY_MS);
		Lpm::idle(min(timeLeft(publishTs, publishPeriodMs), mqtt->getIdleInterval()));
		networkWakeUp(NETWORK_WAKE_UP_DELAY_MS);
		// Determine the next event
		if (timeLeft(publishTs, publishPeriodMs) > mqtt->getIdleInterval()) {
			// Need to call yield
			while(mqtt->isConnected() && mqtt->getIdleInterval() < MQTT_COMMAND_TIMEOUT_MS) {
				mqtt->yield();
			}
		}
	}
}

void buildMessagePayload(char* buffer, int size) {
	const int NUMBER_OF_ROOT_PARAMETERS = 3;
	const int NUMBER_OF_SENSORS = 2;
	const int NUMBER_OF_SENSORS_PARAMETERS = 1;
	// See JSON_OBJECT_SIZE and JSON_ARRAY_SIZE to predict buffer size
	// Let's make double the prediction
	const int BUFFER_SIZE = 2 * (JSON_OBJECT_SIZE(NUMBER_OF_ROOT_PARAMETERS)
			+ JSON_ARRAY_SIZE(NUMBER_OF_SENSORS)
			+ NUMBER_OF_SENSORS*JSON_OBJECT_SIZE(NUMBER_OF_SENSORS_PARAMETERS));
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
	// Write to buffer
	root.printTo(buffer, size);
}

void check() {
	LOG_PRINTFLN("#################################");
	LOG_PRINTFLN("###      Periodic check       ###");
	LOG_PRINTFLN("### Memory Free :    %.5u B  ###", freeMemory());
	LOG_PRINTFLN("### Time        : %.8lu Ms ###", time.millis());
	LOG_PRINTFLN("### Period      : %.8lu Ms ###", publishPeriodMs);
	LOG_PRINTFLN("#################################");
}

void connect() {
	int rc = network->connect(MQTT_HOST, MQTT_PORT);
	if (rc != 0) {
		LOG_PRINTFLN("ERROR, TCP connect, rc:%i", rc);
	} else {
		LOG_PRINTFLN("Connect");
		MqttClient::Error::type rc = MqttClient::Error::SUCCESS;
		MqttClient::ConnectResult connectResult;
		{
			MQTTPacket_connectData options = MQTTPacket_connectData_initializer;
			options.MQTTVersion = 4;
			options.clientID.cstring = (char*)MQTT_CLIENT_ID;
			options.cleansession = true;
			options.keepAliveInterval = MQTT_KEEP_ALIVE_INTERVAL_SEC;
			rc = mqtt->connect(options, connectResult);
		}
		if (rc != MqttClient::Error::SUCCESS) {
			LOG_PRINTFLN("ERROR, Connect, rc:%i", rc);
		} else {
			const char* topic = MQTT_SUBSCRIBE_TOPIC_CFG;
			LOG_PRINTFLN("Subscribe to %s", topic);
			rc = mqtt->subscribe(topic, MQTT_SUBSCRIBE_QOS, processMessageConfig);
			if (rc != MqttClient::Error::SUCCESS) {
				LOG_PRINTFLN("ERROR, Subscribe, rc:%i", rc);
				LOG_PRINTFLN("Drop connection");
				mqtt->disconnect();
			} else {
				// Success
				return;
			}
		}
	}
	// Failure => disconnect
	network->disconnect();
}

void processMessageConfig(MqttClient::MessageData& md) {
	const MqttClient::Message& msg = md.message;
	char payload[msg.payloadLen + 1];
	memcpy(payload, msg.payload, msg.payloadLen);
	payload[msg.payloadLen] = '\0';
	LOG_PRINTFLN(
		"Configuration arrived: retained %u, payload:[%s]",
		msg.retained, payload
	);
	const int NUMBER_OF_ROOT_PARAMETERS = 1;
	// See JSON_OBJECT_SIZE to predict buffer size
	// Let's make double the prediction
	const int BUFFER_SIZE = 2 * (JSON_OBJECT_SIZE(NUMBER_OF_ROOT_PARAMETERS));
	StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject((char*)payload);
	// Check if parsing succeeds
	if (!root.success()) {
		LOG_PRINTFLN("ERROR, Can't pars configuration");
		return;
	}
	publishPeriodMs = root["period"];
}

void networkHibernate(unsigned long delay) {
	Lpm::idle(delay);
	digitalWrite(PIN_LPM_NETWORK, HIGH);
}

void networkWakeUp(unsigned long delay) {
	digitalWrite(PIN_LPM_NETWORK, LOW);
	Lpm::idle(delay);
}

