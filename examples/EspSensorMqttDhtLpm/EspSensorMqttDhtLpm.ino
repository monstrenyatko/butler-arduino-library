/* System Includes */
#include <Arduino.h>
/* External Includes */
#include <MqttClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <time.h>
/* Internal Includes */
#include <ButlerArduinoLibrary.h>
#include <ButlerArduinoTime.hpp>
#include <ButlerArduinoLogger.hpp>
#include <ButlerArduinoDhtSensor.hpp>
#include <ButlerArduinoHwUart.hpp>
#include <ButlerArduinoEspLpm.hpp>
#include <ButlerArduinoEspTime.hpp>
#include <ButlerArduinoSensorLoopOneShot.hpp>
#include <ButlerArduinoEspStorage.hpp>
#include <ButlerArduinoEspWiFiConfigCaptivePortal.hpp>
#include <ButlerArduinoHeapArrayBuffer.hpp>
#include <ButlerArduinoStrings.hpp>
#include <ButlerArduinoJsonConfig.hpp>
#include <ButlerArduinoWiFiJsonConfig.hpp>
#include <ButlerArduinoAuthJsonConfig.hpp>
#include <ButlerArduinoEspManager.hpp>


////////// CONFIGURATION //////////
ADC_MODE(ADC_VCC);
#define DHTTYPE										DHT11
#define PIN_DHT										2
#define MQTT_MAX_PACKET_SIZE						128
#define MQTT_MAX_PAYLOAD_SIZE						96
#define MQTT_MAX_MESSAGE_HANDLERS					1
#define MQTT_COMMAND_TIMEOUT_MS						(3*1000L)
#define MQTT_KEEP_ALIVE_INTERVAL_SEC				(lCtx.publishPeriodMs/1000L*2)
#define MQTT_SUBSCRIBE_QOS							MqttClient::QOS0
#define MQTT_PUBLISH_QOS							MqttClient::QOS0
#define MQTT_LISTEN_TIME_MS							(1*1000L)


////////// DECLARATION //////////

//// APP SPECIFIC CONFIGURATION ////
struct AppJsonConfig: public Butler::Arduino::Config::JsonConfigNode {
	//// PERSISTENCES ////
	uint32_t										period = (1*60*1000L);

	bool decode(JsonObject &json) {
		bool updated = false;
		// PERIOD
		if (json.containsKey(Butler::Arduino::Strings::PERIOD)) {
			uint32_t v = json[Butler::Arduino::Strings::PERIOD];
			if (v != period) {
				period = v;
				updated = true;
			}
		}
		return updated;
	}

	void encode(JsonObject &json, JsonBuffer &jsonBuffer) {
		// PERIOD
		json[Butler::Arduino::Strings::PERIOD] = period;
	}
};

//// CONFIGURATION ////
struct Configuration: public Butler::Arduino::Config::JsonConfig<2> {
	//// CONSTANTS ////
	static constexpr uint32_t						HW_UART_SPEED = 115200L;
	static constexpr uint8_t						CFG_RESET_PIN = 13;
	static constexpr uint32_t						CFG_RESET_DELAY_MS = (4*1000L);
	static constexpr uint32_t						NET_CONNECT_TM_MS = (10*1000L);
	static constexpr uint32_t						NET_CONNECT_ERROR_RETRY_TM_MS = (2*60*1000L);
	static constexpr uint32_t						IDLE_TM_MS = (2*60*1000L);
	static constexpr const char*					APP_NAMESPACE = "butler";
	static constexpr const char*					APP_GROUP = "sensor";
	static constexpr const char*					NAME_PREFIX = "BUTLER-";
	static constexpr const char*					SERVER_ADDR = "butler";
	static constexpr uint16_t						SERVER_HTTP_PORT = 8000;
	static constexpr uint16_t						SERVER_HTTPS_PORT = 8443;
	static constexpr uint16_t						SERVER_MQTT_PORT = 1883;
	static constexpr uint16_t						SERVER_MQTTS_PORT = 8883;
	//// GENERATED ////
	String											mqttTopicConfig;
	String											mqttTopicData;
	//// PERSISTENCES ////
	Butler::Arduino::Config::WiFiJsonConfig			wifi;
	Butler::Arduino::Config::AuthJsonConfig			auth;
	AppJsonConfig									app;

	Configuration() {
		addNode(Butler::Arduino::Strings::WIFI, wifi);
		addNode(Butler::Arduino::Strings::AUTH, auth);
		addNode(Butler::Arduino::Strings::APP, app);
	}
};

//// SLEEP PERSISTENCES ////
struct IdleMemory {

} __attribute__((aligned(4)));

////////// OBJECTS //////////
IdleMemory											idleMemory;
Butler::Arduino::EspManager<Configuration>			manager;
Butler::Arduino::LoopContext						lCtx;
Butler::Arduino::LoopConstants						lConst;
Butler::Arduino::DhtSensor							sensor(*new DHT(PIN_DHT, DHTTYPE));
WiFiClient											network;
//WiFiClientSecure									network;

////////// IMPLEMENTATION //////////
void buildMessagePayload(char* buffer, int size) {
	// Get sensor values
	Butler::Arduino::SensorValue vTemp = sensor.getTemperature();
	Butler::Arduino::SensorValue vHumid = sensor.getHumidity();
	// Predict buffer size
	const int NUMBER_OF_ROOT_PARAMETERS = 3;
	const int NUMBER_OF_SENSORS = 2;
	const int NUMBER_OF_SENSORS_PARAMETERS = 1;
	const int BUFFER_SIZE =
		JSON_OBJECT_SIZE(NUMBER_OF_ROOT_PARAMETERS)								// root
		+ JSON_ARRAY_SIZE(NUMBER_OF_SENSORS)									// data array
		+ JSON_OBJECT_SIZE(NUMBER_OF_SENSORS_PARAMETERS)*NUMBER_OF_SENSORS;		// data array elements
	// Allocate buffer
	DynamicJsonBuffer jsonBuffer(BUFFER_SIZE);
	// Encode message
	JsonArray &data = jsonBuffer.createArray();
	if (sensor.verify(vTemp)) {
		JsonObject &obj = jsonBuffer.createObject();
		obj[Butler::Arduino::Strings::PAYLOAD_KEY_SENSOR_DATA_TYPE_TEMPERATURE] = vTemp;
		data.add(obj);
	}
	if (sensor.verify(vHumid)) {
		JsonObject &obj = jsonBuffer.createObject();
		obj[Butler::Arduino::Strings::PAYLOAD_KEY_SENSOR_DATA_TYPE_HUMIDITY] = vHumid;
		data.add(obj);
	}
	JsonObject &root = jsonBuffer.createObject();
	root[Butler::Arduino::Strings::PAYLOAD_KEY_VERSION] = 1;
	root[Butler::Arduino::Strings::PAYLOAD_KEY_ID] = lConst.id;
	root[Butler::Arduino::Strings::PAYLOAD_KEY_DATA] = data;
	// Write to buffer
	root.printTo(buffer, size);
}

void processMessageConfig(MqttClient::MessageData& md) {
	bool changed = false;
	// Process
	{
		const MqttClient::Message &msg = md.message;
		char payload[msg.payloadLen + 1];
		memcpy(payload, msg.payload, msg.payloadLen);
		payload[msg.payloadLen] = '\0';
		LOG_PRINTFLN(manager.getContext(), "Configuration arrived: %s", payload);
		changed = manager.getConfig().update(manager.getContext(), payload);
	}
	// Apply
	if (changed) {
		LOG_PRINTFLN(manager.getContext(), "Configuration changed");
		manager.getConfig().store(manager.getContext(), manager.getConfigStorage());
		manager.restart();
	}
}

void setup() {
	manager.setup();
	//// LOOP CTX ////
	lCtx.publishPeriodMs = manager.getConfig().app.period;
	//// MQTT TOPIC ////
	manager.getConfig().mqttTopicConfig = Butler::Arduino::Util::makeTopic(
			Butler::Arduino::Strings::TOPIC_MODEL_CONFIG,
			manager.getConfig().APP_NAMESPACE,
			manager.getConfig().APP_GROUP,
			manager.getId()
	);
	manager.getConfig().mqttTopicData = Butler::Arduino::Util::makeTopic(
			Butler::Arduino::Strings::TOPIC_MODEL_DATA,
			manager.getConfig().APP_NAMESPACE,
			manager.getConfig().APP_GROUP,
			manager.getId()
	);
	//// MQTT ////
	{
		class SystemImpl: public MqttClient::System {
		public:
			unsigned long millis() const {
				return manager.getClock().millis();
			}

			void yield(void) {
				::yield();
			}
		};
		MqttClient::System *mqttSystem = new SystemImpl;
		MqttClient::Logger *mqttLogger = new MqttClient::LoggerImpl<HardwareSerial>(Serial);
		MqttClient::Network *mqttNetwork = new MqttClient::NetworkClientImpl<WiFiClient>(network, *mqttSystem);
		MqttClient::Buffer *mqttSendBuffer = new MqttClient::ArrayBuffer<MQTT_MAX_PACKET_SIZE>();
		MqttClient::Buffer *mqttRecvBuffer = new MqttClient::ArrayBuffer<MQTT_MAX_PACKET_SIZE>();
		MqttClient::MessageHandlers *mqttMessageHandlers = new MqttClient::MessageHandlersImpl<MQTT_MAX_MESSAGE_HANDLERS>();
		MqttClient::Options options;
		options.commandTimeoutMs = MQTT_COMMAND_TIMEOUT_MS;
		lCtx.mqtt = new MqttClient(
			options, *mqttLogger, *mqttSystem, *mqttNetwork, *mqttSendBuffer,
			*mqttRecvBuffer, *mqttMessageHandlers
		);
	}
	//// LOOP SETUP ////
	lConst.id = manager.getId().c_str();
	lConst.keepAlivePeriodSec = MQTT_KEEP_ALIVE_INTERVAL_SEC;
	lConst.publishPayloadMaxSize = MQTT_MAX_PAYLOAD_SIZE;
	lConst.publishTopic = manager.getConfig().mqttTopicData.c_str();
	lConst.publishQoS = MQTT_PUBLISH_QOS;
	lConst.configTopic = manager.getConfig().mqttTopicConfig.c_str();
	lConst.configQoS = MQTT_SUBSCRIBE_QOS;
	lConst.configListenPeriodMs = MQTT_LISTEN_TIME_MS;
	lConst.buildMessagePayload = buildMessagePayload;
	lConst.processConfigMessage = processMessageConfig;
	Butler::Arduino::Loop::setup(manager.getContext(), lCtx, lConst);
	////// INIT END //////
	LOG_PRINTFLN(manager.getContext(), "#################################");
	LOG_PRINTFLN(manager.getContext(), "###    Butler Sensor");
	LOG_PRINTFLN(manager.getContext(), "#################################");
}

void loop() {
	manager.printState();
	LOG_PRINTFLN(manager.getContext(), "### VCC         : %u", (unsigned int)((ESP.getVcc()/1024.00f)*1000));
	LOG_PRINTFLN(manager.getContext(), "### Period      : %lu Ms", lCtx.publishPeriodMs);
	LOG_PRINTFLN(manager.getContext(), "#################################");
	manager.waitNetwork();
	manager.waitNtpTime();
	manager.checkServerFingerprintsUpdate();
	manager.authenticate();
	manager.checkFirmwareUpdate();
	// Establish TCP connection
	manager.connectServer(network, manager.getConfig().SERVER_ADDR, 1883);
	// Action
	Butler::Arduino::LoopStatus::type loopStatus = Butler::Arduino::Loop::loop(manager.getContext(), lCtx, lConst);
	// Idle
	unsigned long idlePeriodMs = (loopStatus == Butler::Arduino::LoopStatus::CONNECTION_FAILURE)
			? manager.getConfig().NET_CONNECT_ERROR_RETRY_TM_MS
			: Butler::Arduino::Time::calcTimeLeft(manager.getClock().millis(), 0, lCtx.publishPeriodMs);
	manager.idle(idlePeriodMs);
}
