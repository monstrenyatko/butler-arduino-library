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

/* System Includes */
#include <Arduino.h>
/* External Includes */
#include <MemoryFree.h>
#include <MqttClient.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <DHT.h>
/* Internal Includes */
#include <ButlerArduinoLibrary.h>
#include <ButlerArduinoLogger.hpp>
#include <ButlerArduinoNetwork.hpp>
#include <ButlerArduinoUartNetwork.hpp>
#include <ButlerArduinoDhtSensor.hpp>
#include <ButlerArduinoAvrLpm.hpp>
#include <ButlerArduinoSwUart.hpp>
#include <ButlerArduinoHwUart.hpp>
#include <ButlerArduinoSensorLoop.hpp>


////////// CONFIGURATION //////////
#define ID											"TEST_SENSOR_ID"
#define DOMAIN										"test"
#define DHTTYPE										DHT11
#define PIN_DHT										2
#define PIN_DHT_ON									4
#define PIN_SW_UART_RX								10
#define PIN_SW_UART_TX								11
#define PIN_LPM_NETWORK								9
#define PIN_LED_AWAKE								13
#define MQTT_HOST									"STUB"
#define MQTT_PORT									0
#define MQTT_MAX_PACKET_SIZE						128
#define MQTT_MAX_PAYLOAD_SIZE						96
#define MQTT_MAX_MESSAGE_HANDLERS					1
#define MQTT_COMMAND_TIMEOUT_MS						(3*1000L)
#define MQTT_KEEP_ALIVE_INTERVAL_SEC				(lCtx.publishPeriodMs/1000L*2)
#define MQTT_SUBSCRIBE_QOS							MqttClient::QOS0
#define MQTT_SUBSCRIBE_TOPIC_CFG					DOMAIN "/sensor/" ID "/config"
#define MQTT_PUBLISH_QOS							MqttClient::QOS0
#define MQTT_PUBLISH_TOPIC							DOMAIN "/sensor/" ID "/data"
#define MQTT_LISTEN_TIME_MS							(3*1000L)
#define MQTT_PUBLISH_PERIOD_MS						(1*60*1000L)
#define MQTT_UPDATE_CONFIG_PERIOD_MS				(lCtx.publishPeriodMs*3L)
#define MQTT_CONNECT_RETRIES_QTY					5
#define MQTT_DISCONNECTED_IDLE_PERIOD_MS			(2*60*1000L)
#define NETWORK_HIBERNATE_DELAY_MS					10
#define NETWORK_WAKE_UP_DELAY_MS					10
#define LPM_MODE									Butler::Arduino::LPM_MODE_PWR_DOWN
#define HW_UART_SPEED								57600L
#define SW_UART_SPEED								9600L

////////// OBJECTS DECLARATION //////////
class SystemImpl: public MqttClient::System, public Butler::Arduino::Time::Clock {
public:
	unsigned long millis() const {
		return ::millis();
	}
};

////////// OBJECTS //////////
Butler::Arduino::Context							gCtx;
Butler::Arduino::LoopContext						lCtx;
Butler::Arduino::LoopConstants						lConst;
Butler::Arduino::Network							*network = NULL;
Butler::Arduino::DhtSensor							sensor(*new DHT(PIN_DHT, DHTTYPE));

////////// IMPLEMENTATION //////////
void initLoopConstants(Butler::Arduino::LoopConstants& lConst);

void check() {
	LOG_PRINTFLN(gCtx, "#################################");
	LOG_PRINTFLN(gCtx, "###      Periodic check       ###");
	LOG_PRINTFLN(gCtx, "### Memory Free :    %.5u B  ###", freeMemory());
	LOG_PRINTFLN(gCtx, "### Time        : %.8lu Ms ###", gCtx.time->millis());
	LOG_PRINTFLN(gCtx, "### Period      : %.8lu Ms ###", lCtx.publishPeriodMs);
	LOG_PRINTFLN(gCtx, "#################################");
}

void buildMessagePayload(char* buffer, int size) {
	// Get sensor values
	digitalWrite(PIN_DHT_ON, HIGH);
	Butler::Arduino::SensorValue vTemp = sensor.getTemperature();
	Butler::Arduino::SensorValue vHumid = sensor.getHumidity();
	digitalWrite(PIN_DHT_ON, LOW);
	// Predict buffer size
	const int NUMBER_OF_ROOT_PARAMETERS = 3;
	const int NUMBER_OF_SENSORS = 2;
	const int NUMBER_OF_SENSORS_PARAMETERS = 1;
	const int BUFFER_SIZE =
		JSON_OBJECT_SIZE(NUMBER_OF_ROOT_PARAMETERS)								// root
		+ JSON_ARRAY_SIZE(NUMBER_OF_SENSORS)									// data array
		+ JSON_OBJECT_SIZE(NUMBER_OF_SENSORS_PARAMETERS)*NUMBER_OF_SENSORS;		// data array elements
	// Allocate buffer
	StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
	// Encode message
	JsonArray& data = jsonBuffer.createArray();
	if (sensor.verify(vTemp)) {
		JsonObject& obj = jsonBuffer.createObject();
		obj["temp"] = vTemp;
		data.add(obj);
	}
	if (sensor.verify(vHumid)) {
		JsonObject& obj = jsonBuffer.createObject();
		obj["humid"] = vHumid;
		data.add(obj);
	}
	JsonObject& root = jsonBuffer.createObject();
	root["v"] = 1;
	root["id"] = ID;
	root["data"] = data;
	// Write to buffer
	root.printTo(buffer, size);
}

void processMessageConfig(MqttClient::MessageData& md) {
	bool changed = false;
	const MqttClient::Message& msg = md.message;
	char payload[msg.payloadLen + 1];
	memcpy(payload, msg.payload, msg.payloadLen);
	payload[msg.payloadLen] = '\0';
	LOG_PRINTFLN(gCtx, "Configuration arrived: %s", payload);
	// Predict buffer size
	const int NUMBER_OF_ROOT_PARAMETERS = 1;
	const int BUFFER_SIZE = JSON_OBJECT_SIZE(NUMBER_OF_ROOT_PARAMETERS);
	DynamicJsonBuffer jsonBuffer(BUFFER_SIZE);
	JsonObject& root = jsonBuffer.parseObject(&payload[0]);
	// Check if parsing succeeds
	if (!root.success()) {
		LOG_PRINTFLN(gCtx, "ERROR, Can't pars configuration");
		return;
	}
	// Reconnect if publishPeriodMs is changed to update keep-alive timer
	{
		unsigned long old = lCtx.publishPeriodMs;
		lCtx.publishPeriodMs = root["period"];
		changed = (old != lCtx.publishPeriodMs);
	}
	// TODO: Store publishPeriodMs to EEPROM
	if (changed) {
		LOG_PRINTFLN(gCtx, "Configuration changed => reconnect");
		initLoopConstants(lConst);
		lCtx.mqtt->disconnect();
	}
}

int networkConnect() {
	return network->connect(MQTT_HOST, MQTT_PORT);
}

void networkDisconnect() {
	network->disconnect();
}

void networkHibernate() {
	gCtx.lpm->idle(NETWORK_HIBERNATE_DELAY_MS);
	digitalWrite(PIN_LPM_NETWORK, HIGH);
}

void networkWakeUp() {
	digitalWrite(PIN_LPM_NETWORK, LOW);
	gCtx.lpm->idle(NETWORK_WAKE_UP_DELAY_MS);
}

void initLoopConstants(Butler::Arduino::LoopConstants& lConst) {
	lConst.id = ID;
	lConst.connectAttemptsMaxQty = MQTT_CONNECT_RETRIES_QTY;
	lConst.keepAlivePeriodSec = MQTT_KEEP_ALIVE_INTERVAL_SEC;
	lConst.disconnectedIdlePeriodMs = MQTT_DISCONNECTED_IDLE_PERIOD_MS;
	lConst.publishPayloadMaxSize = MQTT_MAX_PAYLOAD_SIZE;
	lConst.publishTopic = MQTT_PUBLISH_TOPIC;
	lConst.publishQoS = MQTT_PUBLISH_QOS;
	lConst.configUpdatePeriodMs = MQTT_UPDATE_CONFIG_PERIOD_MS;
	lConst.configTopic = MQTT_SUBSCRIBE_TOPIC_CFG;
	lConst.configQoS = MQTT_SUBSCRIBE_QOS;
	lConst.configListenPeriodMs = MQTT_LISTEN_TIME_MS;
	lConst.commandTimeoutMs = MQTT_COMMAND_TIMEOUT_MS;
	//
	lConst.reset = 0; // Declare reset function at address 0
	lConst.networkConnect = networkConnect;
	lConst.networkDisconnect = networkDisconnect;
	lConst.networkHibernate = networkHibernate;
	lConst.networkWakeUp = networkWakeUp;
	lConst.buildMessagePayload = buildMessagePayload;
	lConst.processConfigMessage = processMessageConfig;
}

/** Called once at startup */
void setup() {
	//// RESET ////
	gCtx = Butler::Arduino::Context();
	lCtx = Butler::Arduino::LoopContext();
	lConst = Butler::Arduino::LoopConstants();

	//// SYSTEM ////
	SystemImpl *system = new SystemImpl;

	//// TIME ////
	gCtx.time = system;

	//// HW UART ////
	Butler::Arduino::Uart *hwUart = new Butler::Arduino::HwUart({HW_UART_SPEED});

	//// SF UART ////
	Butler::Arduino::Uart *swUart = new Butler::Arduino::SwUart({SW_UART_SPEED, PIN_SW_UART_RX, PIN_SW_UART_TX});

	//// LOG ////
	gCtx.logger = hwUart;

	//// LPM ////
	{
		Butler::Arduino::AvrLpmConfig config;
		config.pinLedAwake = PIN_LED_AWAKE;
		config.mode = LPM_MODE;
		gCtx.lpm = new Butler::Arduino::AvrLpm(config);
	}

	//// NETWORK ////
	pinMode(PIN_LPM_NETWORK, OUTPUT);
	network = new Butler::Arduino::UartNetwork(*swUart);

	//// SENSORS ////
	pinMode(PIN_DHT_ON, OUTPUT);
	digitalWrite(PIN_DHT_ON, LOW);
	sensor.start();

	//// LOOP CTX ////
	// TODO: Get publishPeriodMs from EEPROM
	lCtx.publishPeriodMs = MQTT_PUBLISH_PERIOD_MS;

	//// MQTT ////
	{
		MqttClient::Logger *mqttLogger = new MqttClient::LoggerImpl<Butler::Arduino::Uart>(*hwUart);
		MqttClient::Network *mqttNetwork = new MqttClient::NetworkImpl<Butler::Arduino::Network>(*network, *system);
		MqttClient::Buffer *mqttSendBuffer = new MqttClient::ArrayBuffer<MQTT_MAX_PACKET_SIZE>();
		MqttClient::Buffer *mqttRecvBuffer = new MqttClient::ArrayBuffer<MQTT_MAX_PACKET_SIZE>();
		MqttClient::MessageHandlers *mqttMessageHandlers = new MqttClient::MessageHandlersImpl<MQTT_MAX_MESSAGE_HANDLERS>();
		MqttClient::Options options;
		options.commandTimeoutMs = MQTT_COMMAND_TIMEOUT_MS;
		lCtx.mqtt = new MqttClient(
			options, *mqttLogger, *system, *mqttNetwork, *mqttSendBuffer,
			*mqttRecvBuffer, *mqttMessageHandlers
		);
	}

	//// LOOP SETUP ////
	initLoopConstants(lConst);
	Butler::Arduino::Loop::setup(gCtx, lCtx, lConst);

	////// INIT END //////
	LOG_PRINTFLN(gCtx, "#################################");
	LOG_PRINTFLN(gCtx, "###      Butler Sensor        ###");
	LOG_PRINTFLN(gCtx, "### ID          : %11s ###", ID);
	LOG_PRINTFLN(gCtx, "#################################");
}

/** Endless main loop */
void loop() {
	check();
	Butler::Arduino::Loop::loop(gCtx, lCtx, lConst);
}

