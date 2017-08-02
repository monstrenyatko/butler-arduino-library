/*
 *******************************************************************************
 *
 * Purpose: Sensor application loop implementation.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_LOOP_H_
#define BUTLER_ARDUINO_LOOP_H_

/* System Includes */
#include <MqttClient.h>
/* Internal Includes */
#include "ButlerArduinoContext.hpp"
#include "ButlerArduinoTime.hpp"
#include "ButlerArduinoUtil.hpp"
#include "ButlerArduinoLogger.hpp"


namespace Butler {
namespace Arduino {

struct LoopStatus {
	typedef int										type;
	enum {
		CONNECTION_FAILURE = -2,
		FAILURE = -1,
		SUCCESS = 0
	};
};

struct LoopConstants {
	// Types
	typedef void (*MessagePayloadBuilder_f)(char* buffer, int size);
	typedef void (*ConfigMessageProcessor_f)(MqttClient::MessageData& md);

	// Constants
	const char*											id = NULL;
	unsigned long										keepAlivePeriodSec = 0;
	int													publishPayloadMaxSize = 0;
	const char*											publishTopic = NULL;
	MqttClient::QoS										publishQoS = MqttClient::QOS0;
	const char*											configTopic = NULL;
	MqttClient::QoS										configQoS = MqttClient::QOS0;
	unsigned long										configListenPeriodMs = 0;

	// Functions
	MessagePayloadBuilder_f								buildMessagePayload = NULL;
	ConfigMessageProcessor_f							processConfigMessage = NULL;
};

struct LoopContext {
	// Resources
	MqttClient											*mqtt = NULL;

	// Configuration
	unsigned long										publishPeriodMs = 0;
};

namespace LoopPrivate {

void connect(Context& gCtx, LoopContext& lCtx, const LoopConstants& lConst) {
	MqttClient::Error::type rc = MqttClient::Error::SUCCESS;
	MqttClient::ConnectResult connectResult;
	{
		MQTTPacket_connectData options = MQTTPacket_connectData_initializer;
		options.MQTTVersion = 4;
		options.clientID.cstring = (char*)lConst.id;
		options.cleansession = true;
		options.keepAliveInterval = lConst.keepAlivePeriodSec;
		rc = lCtx.mqtt->connect(options, connectResult);
	}
	if (rc == MqttClient::Error::SUCCESS) {
		// Success
	} else {
		// Failure
		LOG_PRINTFLN(gCtx, "ERROR, Connect, rc:%i", rc);
	}
}

void updateConfig(Context& gCtx, LoopContext& lCtx, const LoopConstants& lConst) {
	MqttClient::Error::type rc = lCtx.mqtt->subscribe(
		lConst.configTopic, lConst.configQoS, lConst.processConfigMessage
	);
	if (rc == MqttClient::Error::SUCCESS) {
		// Listen for configuration
		lCtx.mqtt->yield(lConst.configListenPeriodMs);
	} else {
		LOG_PRINTFLN(gCtx,"ERROR, Subscribe, rc:%i", rc);
	}
}

} // Private

namespace Loop {

inline void setup(Context& gCtx, LoopContext& lCtx, const LoopConstants& lConst) {

}

inline LoopStatus::type loop(Context& gCtx, LoopContext& lCtx, const LoopConstants& lConst) {
	LoopStatus::type res = LoopStatus::SUCCESS;
	// Connecting
	LoopPrivate::connect(gCtx, lCtx, lConst);
	if (!lCtx.mqtt->isConnected()) {
		return LoopStatus::CONNECTION_FAILURE;
	}
	// Update configuration
	LoopPrivate::updateConfig(gCtx, lCtx, lConst);
	// Prepare data for Publish
	const int bufferSize = lConst.publishPayloadMaxSize;
	char buffer[bufferSize];
	memset(buffer, 0, bufferSize);
	// Build message payload
	lConst.buildMessagePayload(buffer, bufferSize);
	// Build message
	MqttClient::Message message;
	message.qos = lConst.publishQoS;
	message.retained = false;
	message.dup = false;
	message.payload = (void*) buffer;
	message.payloadLen = strlen(buffer) + 1;
	// Publish
	MqttClient::Error::type rc = lCtx.mqtt->publish(lConst.publishTopic, message);
	if (rc != MqttClient::Error::SUCCESS) {
		LOG_PRINTFLN(gCtx, "ERROR, Publish, rc:%i", rc);
		res = LoopStatus::FAILURE;
	}
	return lCtx.mqtt->isConnected() ? res : LoopStatus::CONNECTION_FAILURE;
}

} // Loop

}}
#endif // BUTLER_ARDUINO_LOOP_H_

