/* Internal Includes */
#include "Logger.h"
#include "Network.h"
#include "UartNetwork.h"
#include "SensorTemperature.h"
#include "SensorLight.h"
#include "GoliathMqttSensor.h"
/* External Includes */
#include <MemoryFree.h>
#include <Countdown.h>
#include <MQTTClient.h>
/* System Includes */
#include <SoftwareSerial.h>

////////// CONFIGURATION //////////
#define MQTT_MAX_PACKET_SIZE 80
#define MQTT_MAX_MESSAGE_HANDLERS 1
#define MQTT_HOST "test.mosquitto.org"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "GOLIATH_SENSOR_TEMPERATURE"
#define MQTT_SUBSCRIBE_QOS MQTT::QOS1
#define MQTT_SUBSCRIBE_TOPIC_CFG "goliath/cfg/sensor/temperature"
#define MQTT_PUBLISH_QOS MQTT::QOS1
#define MQTT_PUBLISH_TOPIC "goliath/sensor/home/1234"
#define MQTT_PUBLISH_PERIOD_MS 15000
#define MQTT_PUBLISH_PERIOD_MAX_MS (1*60*60*1000) // 1 hour
#define SENSOR_LIGHT_PIN A3

////////// OBJECTS //////////
Print* logPrinter = NULL;
Network* network = NULL;
MQTT::Client<Network, Countdown, MQTT_MAX_PACKET_SIZE, MQTT_MAX_MESSAGE_HANDLERS>* mqtt =
		NULL;
Sensor* sensorTemperature = NULL;
Sensor* sensorLight = NULL;
unsigned long publishPeriodMs = MQTT_PUBLISH_PERIOD_MS;

//// DECLARATION ////
void check();
void connect();
void processMessageCfg(MQTT::MessageData& md);

////////// IMPLEMENTATION //////////
/**
 * Called once at startup
 */
void setup() {
	////// INIT //////
	//// LOG ////
	{
		SoftwareSerial* t = new SoftwareSerial(10, 11); // Pins RX, TX
		// set the data rate
		t->begin(4800);
		logPrinter = t;
	}
	Logger::init(*logPrinter);

	//// NETWORK ////
	{
		UartNetwork::init();
		network = new UartNetwork;
	}

	//// MQTT ////
	{
		mqtt = new MQTT::Client<Network, Countdown, MQTT_MAX_PACKET_SIZE,
		MQTT_MAX_MESSAGE_HANDLERS>(*network);
	}

	//// SENSOR ////
	{
		sensorTemperature = new SensorTemperature;
		sensorLight = new SensorLight(SENSOR_LIGHT_PIN);
	}

	////// INIT END //////
	LOG_PRINTFLN("#################################");
	LOG_PRINTFLN("### MQTT Client for Goliath's ###");
	LOG_PRINTFLN("###       sensor started      ###");
	LOG_PRINTFLN("#################################");
}

/**
 * Endless main loop
 */
void loop() {
	check();
	while (!mqtt->isConnected()) {
		connect();
	}
	{
		const uint8_t bufSize = 128;
		char buf[bufSize];
		memset(buf, 0, bufSize);
		// format is "name1:value,name2:value,nameN:value"
		snprintf_P(buf, bufSize, PSTR("temperature:%li,light:%li"),
				sensorTemperature->getData(), sensorLight->getData());
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
	LOG_PRINTFLN("#################################");
	LOG_PRINTFLN("###      Periodic check       ###");
	LOG_PRINTFLN("### Memory Free :    %.5u B  ###", freeMemory());
	LOG_PRINTFLN("### Period      : %.8lu Ms ###", publishPeriodMs);
	LOG_PRINTFLN("#################################");
}

void connect() {
	//LOG_PRINTFLN("TCP connect to %s:%d", MQTT_HOST, MQTT_PORT);
	int rc = network->connect(MQTT_HOST, MQTT_PORT);
	if (rc != 0) {
		LOG_PRINTFLN("ERROR, TCP connect, rc:%d", rc);
		return;
	}
	LOG_PRINTFLN("MQTT connect");
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.MQTTVersion = 3;
	data.clientID.cstring = MQTT_CLIENT_ID;
	rc = mqtt->connect(data);
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
				MQTT_PUBLISH_PERIOD_MAX_MS :
																		tmp;
			}
		}
		// Find the next token
		token = strtok(0, tokenSep);
	}
}
