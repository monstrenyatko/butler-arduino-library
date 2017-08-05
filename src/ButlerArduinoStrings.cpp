/*
 *******************************************************************************
 *
 * Purpose: String constants.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

/* System Includes */
/* Internal Includes */
#include "ButlerArduinoStrings.hpp"


namespace Butler {
namespace Arduino {

namespace Strings {

const char EMPTY[] = "";
const char APP[] = "app";
const char WIFI[] = "wifi";
const char AUTH[] = "auth";
const char PERIOD[] = "period";
const char SSID[] = "ssid";
const char PASSPHRASE[] = "passphrase";
const char PAIRED[] = "paired";
const char FINGERPRINTS[] = "fingerprints";
const char TOKEN[] = "token";

const char PAYLOAD_KEY_VERSION[] = "v";
const char PAYLOAD_KEY_ID[] = "id";
const char PAYLOAD_KEY_DATA[] = "data";
const char PAYLOAD_KEY_RESULTS[] = "results";
const char PAYLOAD_KEY_VALUE[] = "value";
const char PAYLOAD_KEY_SENSOR_DATA_TYPE_TEMPERATURE[] = "temp";
const char PAYLOAD_KEY_SENSOR_DATA_TYPE_HUMIDITY[] = "humid";

const char MODEL_KEY_ADDR[] = "<a>";
const char MODEL_KEY_PORT[] = "<p>";
const char MODEL_KEY_NAMESPACE[] = "<ns>";
const char MODEL_KEY_GROUP[] = "<g>";
const char MODEL_KEY_ID[] = "<id>";

const char URL_MODEL_UPDATE_FW[] = "https://<a>:<p>/fw/update/";
const char URL_MODEL_UPDATE_FW_NOT_S[] = "http://<a>:<p>/fw/update/<id>/";
const char URL_MODEL_FINGERPRINTS[] = "https://<a>:<p>/cert/fingerprints/<a>/";
const char URL_MODEL_FINGERPRINTS_NOT_S[] = "http://<a>:<p>/cert/fingerprints/<a>/";

const char TOPIC_MODEL_CONFIG[] = "<ns>/<g>/<id>/config";
const char TOPIC_MODEL_DATA[] = "<ns>/<g>/<id>/data";

} // Strings

}}

