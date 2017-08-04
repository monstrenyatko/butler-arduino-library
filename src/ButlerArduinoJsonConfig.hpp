/*
 *******************************************************************************
 *
 * Purpose: JSON configuration implementation.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_JSON_CONFIG_H_
#define BUTLER_ARDUINO_JSON_CONFIG_H_

/* System Includes */
#include <stdint.h>
#include <ArduinoJson.h>
/* Internal Includes */
#include "ButlerArduinoStrings.hpp"
#include "ButlerArduinoStorage.hpp"
#include "ButlerArduinoLogger.hpp"
#include "ButlerArduinoContext.hpp"
#include "ButlerArduinoHeapArrayBuffer.hpp"


namespace Butler {
namespace Arduino {

namespace Config {

namespace ConfigPrivate {

struct JsonConfigBase {
	virtual ~JsonConfigBase() {}
	virtual bool decode(JsonObject &json) = 0;
	virtual void encode(JsonObject &json, JsonBuffer &jsonBuffer) = 0;
	virtual bool isValid() { return true; }
};

} // ConfigPrivate

struct JsonConfigNode: public ConfigPrivate::JsonConfigBase {
	virtual ~JsonConfigNode() {}
};

template<uint8_t NODE_QTY>
class JsonConfig: public ConfigPrivate::JsonConfigBase {
public:
	JsonConfig() {
		for (int i = 0; i < NODE_QTY; ++i) {
			mNodes[i] = Node();
		}
	}

	virtual ~JsonConfig() {}

	void addNode(const char *name, JsonConfigBase &config) {
		if (name && mEmptyNodeIdx < NODE_QTY) {
			mNodes[mEmptyNodeIdx++] = Node(name, config);
		}
	}

	bool update(Context& gCtx, char jsonString[]) {
		LOG_PRINTFLN(gCtx, "[config] Update");
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(&jsonString[0]);
		// Check if parsing succeeds
		if (!root.success()) {
			LOG_PRINTFLN(gCtx, "[config] ERROR, Can't pars");
			return false;
		}
		return decode(root);
	}

	bool load(Context &gCtx, Storage &storage) {
		LOG_PRINTFLN(gCtx, "[config] Load");
		uint32_t readSize = storage.readSize();
		if (readSize > storage.size()) {
			LOG_PRINTFLN(gCtx, "[config] ERROR, Wrong size: %lu", readSize);
			return false;
		}
		if (readSize == 0) {
			LOG_PRINTFLN(gCtx, "[config] ERROR, Empty");
			return false;
		}
		HeapArrayBuffer buffer(readSize);
		if (!storage.read(buffer)) {
			LOG_PRINTFLN(gCtx, "[config] ERROR, Corrupted");
			return false;
		}
		LOG_PRINTFLN_LONG(gCtx, "[config] %s", reinterpret_cast<char*>(buffer.get()));
		DynamicJsonBuffer jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(buffer.get());
		// Check if parsing succeeds
		if (!root.success()) {
			LOG_PRINTFLN(gCtx, "[config] ERROR, Can't pars");
			return false;
		}
		decode(root);
		return isValid();
	}

	void store(Context &gCtx, Storage &storage) {
		LOG_PRINTFLN(gCtx, "[config] Store");
		Butler::Arduino::HeapArrayBuffer buffer;
		{
			DynamicJsonBuffer jsonBuffer;
			JsonObject& root = jsonBuffer.createObject();
			encode(root, jsonBuffer);
			buffer.resize(root.measureLength() + 1);
			root.printTo(reinterpret_cast<char*>(buffer.get()), buffer.size());
		}
		storage.write(buffer);
	}

protected:
	virtual void encode(JsonObject &json, JsonBuffer &jsonBuffer) {
		for (int i = 0; i < mEmptyNodeIdx; ++i) {
			Node &node = mNodes[i];
			if (node.isValid() && node.mConfig->isValid()) {
				JsonObject &o = jsonBuffer.createObject();
				node.mConfig->encode(o, jsonBuffer);
				json[node.mName] = o;
			}
		}
	}

	virtual bool decode(JsonObject &json) {
		bool updated = false;
		for (int i = 0; i < mEmptyNodeIdx; ++i) {
			Node &node = mNodes[i];
			if (node.isValid()) {
				JsonObject &o = json[node.mName];
				if (JsonObject::invalid() != o) {
					updated = node.mConfig->decode(o) || updated;
				}
			}
		}
		return updated;
	}

	virtual bool isValid() {
		bool valid = true;
		for (int i = 0; valid && i < mEmptyNodeIdx; ++i) {
			Node &node = mNodes[i];
			if (node.isValid()) {
				valid = node.mConfig->isValid();
			}
		}
		return valid;
	}

private:
	struct Node {
		const char										*mName = Strings::EMPTY;
		JsonConfigBase									*mConfig = nullptr;

		Node() {}

		Node(const char *name, JsonConfigBase &config): mName(name), mConfig(&config) {}

		bool isValid() {
			return mConfig && mName && mName != Strings::EMPTY;
		}
	};

private:
	uint8_t											mEmptyNodeIdx = 0;
	Node											mNodes[NODE_QTY];
};

} // Config

}}

#endif // BUTLER_ARDUINO_JSON_CONFIG_H_
