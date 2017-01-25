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


#define BUTLER_ARDUINO_LOOP_CALL(func, ...) if(func) func(##__VA_ARGS__)

namespace Butler {
namespace Arduino {

struct LoopConstants {
	// Types
	typedef void (*MessagePayloadBuilder_f)(char* buffer, int size);
	typedef void (*ConfigMessageProcessor_f)(MqttClient::MessageData& md);
	typedef void (*Reset_f)(void);
	typedef int (*NetworkConnect_f)(void);
	typedef void (*NetworkDisconnect_f)(void);
	typedef void (*NetworkHibernate_f)(void);
	typedef void (*NetworkWakeUp_f)(void);

	// Constants
	const char*											id = NULL;
	int													connectAttemptsMaxQty = 0;
	unsigned long										keepAlivePeriodSec = 0;
	unsigned long										disconnectedIdlePeriodMs = 0;
	int													publishPayloadMaxSize = 0;
	const char*											publishTopic = NULL;
	MqttClient::QoS										publishQoS = MqttClient::QOS0;
	unsigned long										configUpdatePeriodMs = 0;
	const char*											configTopic = NULL;
	MqttClient::QoS										configQoS = MqttClient::QOS0;
	unsigned long										configListenPeriodMs = 0;
	unsigned long										commandTimeoutMs = 0;

	// Functions
	Reset_f												reset = NULL;
	NetworkConnect_f									networkConnect = NULL;
	NetworkDisconnect_f									networkDisconnect = NULL;
	NetworkHibernate_f									networkHibernate = NULL;
	NetworkWakeUp_f										networkWakeUp = NULL;
	MessagePayloadBuilder_f								buildMessagePayload = NULL;
	ConfigMessageProcessor_f							processConfigMessage = NULL;
};

struct LoopContext {
	// Resources
	MqttClient											*mqtt = NULL;

	// Configuration
	unsigned long										publishPeriodMs = 0;

	// State
	int													connectCounter = 0;
	unsigned long										publishTs = 0;
	unsigned long										configUpdateTs = 0;
	bool												firstPublish = false;
	bool												firstConfigUpdate = false;
};

namespace LoopPrivate {

void idle(Context& gCtx, const LoopConstants& lConst, unsigned long ms) {
	BUTLER_ARDUINO_LOOP_CALL(lConst.networkHibernate);
	gCtx.lpm->idle(ms);
	BUTLER_ARDUINO_LOOP_CALL(lConst.networkWakeUp);
}

void connect(Context& gCtx, LoopContext& lCtx, const LoopConstants& lConst) {
	if (0 == lConst.networkConnect()) {
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
		if (rc != MqttClient::Error::SUCCESS) {
			LOG_PRINTFLN(gCtx, "ERROR, Connect, rc:%i", rc);
		} else {
			// Success
			return;
		}
		// Failure => disconnect
		lConst.networkDisconnect();
	}
}

bool updateConfig(Context& gCtx, LoopContext& lCtx, const LoopConstants& lConst) {
	bool res = false;
	if (lCtx.mqtt->isConnected()) {
		MqttClient::Error::type rc = lCtx.mqtt->subscribe(
			lConst.configTopic, lConst.configQoS, lConst.processConfigMessage
		);
		if (rc != MqttClient::Error::SUCCESS) {
			LOG_PRINTFLN(gCtx,"ERROR, Subscribe, rc:%i", rc);
		} else {
			// Listen for configuration
			lCtx.mqtt->yield(lConst.configListenPeriodMs);
			// Configuration change might cause the disconnect
			if (lCtx.mqtt->isConnected()) {
				rc = lCtx.mqtt->unsubscribe(lConst.configTopic);
				if (rc != MqttClient::Error::SUCCESS) {
					LOG_PRINTFLN(gCtx, "ERROR, Unsubscribe, rc:%i", rc);
				} else {
					// Success
					res = true;
				}
			}
		}
		if (!res && lCtx.mqtt->isConnected()) {
			lCtx.mqtt->disconnect();
		}
	}
	return res;
}

} // Private

namespace Loop {

inline void setup(Context& gCtx, LoopContext& lCtx, const LoopConstants& lConst) {
	// Done
	BUTLER_ARDUINO_LOOP_CALL(lConst.networkWakeUp);
}

inline void loop(Context& gCtx, LoopContext& lCtx, const LoopConstants& lConst) {
	if (!lCtx.mqtt->isConnected()) {
		// Clean buffers
		lCtx.mqtt->yield();
		// Connecting
		LoopPrivate::connect(gCtx, lCtx, lConst);
		if (!lCtx.mqtt->isConnected()) {
			if (++lCtx.connectCounter > lConst.connectAttemptsMaxQty) {
				LOG_PRINTFLN(gCtx, "ERROR, Max retries qty has been reached => reset");
				lConst.reset();
				// Execution must stop at this point
			}
			LOG_PRINTFLN(gCtx, "Reconnect in %lu ms", lConst.disconnectedIdlePeriodMs);
			LoopPrivate::idle(gCtx, lConst, lConst.disconnectedIdlePeriodMs);
		} else {
			lCtx.connectCounter = 0;
			// Update configuration immediately after reconnect
			lCtx.firstConfigUpdate = true;
			// Do the publish immediately after reconnect
			lCtx.firstPublish = true;
		}
	} else if (
		lCtx.firstConfigUpdate
		|| Time::isTimePassed(gCtx.time->millis(), lCtx.configUpdateTs, lConst.configUpdatePeriodMs)
	) {
		// Process control messages
		lCtx.mqtt->yield();
		// Time to Update configuration
		if (LoopPrivate::updateConfig(gCtx, lCtx, lConst)) {
			lCtx.configUpdateTs = gCtx.time->millis();
			lCtx.firstConfigUpdate = false;
		}
	} else if (
		lCtx.firstPublish
		|| Time::isTimePassed(gCtx.time->millis(), lCtx.publishTs, lCtx.publishPeriodMs)
	) {
		// Time to Publish
		const int bufferSize = lConst.publishPayloadMaxSize;
		char buffer[bufferSize] = {0};
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
		} else {
			lCtx.publishTs = gCtx.time->millis();
			lCtx.firstPublish = false;
		}
	} else {
		// Idle until next event
		unsigned long nextEventDelay = min(
			Time::calcTimeLeft(gCtx.time->millis(), lCtx.publishTs, lCtx.publishPeriodMs),
			Time::calcTimeLeft(gCtx.time->millis(), lCtx.configUpdateTs, lConst.configUpdatePeriodMs)
		);
		// Sleep
		LOG_PRINTFLN(gCtx, "Idle for %lu ms", min(nextEventDelay, lCtx.mqtt->getIdleInterval()));
		LoopPrivate::idle(gCtx, lConst, min(nextEventDelay, lCtx.mqtt->getIdleInterval()));
		// Determine the next event after wake up
		nextEventDelay = min(
			Time::calcTimeLeft(gCtx.time->millis(), lCtx.publishTs, lCtx.publishPeriodMs),
			Time::calcTimeLeft(gCtx.time->millis(), lCtx.configUpdateTs, lConst.configUpdatePeriodMs)
		);
		if (nextEventDelay > lCtx.mqtt->getIdleInterval()) {
			// Need to process control messages
			while(lCtx.mqtt->isConnected() && lCtx.mqtt->getIdleInterval() < lConst.commandTimeoutMs) {
				lCtx.mqtt->yield();
			}
		}
	}
}

} // Loop

}}

#endif // BUTLER_ARDUINO_LOOP_H_

