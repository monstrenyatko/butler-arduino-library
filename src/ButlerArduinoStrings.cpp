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
const char SPACE[] = " ";
const char APP[] = "app";
const char WIFI[] = "wifi";
const char AUTH[] = "auth";
const char PERIOD[] = "period";
const char SSID[] = "ssid";
const char PASSPHRASE[] = "passphrase";
const char PAIRED[] = "paired";
const char FINGERPRINTS[] = "fingerprints";
const char TOKEN[] = "token";
const char USERNAME[] = "username";
const char PASSWORD[] = "password";

const char PAYLOAD_KEY_VERSION[] = "v";
const char PAYLOAD_KEY_ID[] = "id";
const char PAYLOAD_KEY_DATA[] = "data";
const char PAYLOAD_KEY_RESULTS[] = "results";
const char PAYLOAD_KEY_VALUE[] = "value";
const char PAYLOAD_KEY_SENSOR_DATA_TYPE_TEMPERATURE[] = "temp";
const char PAYLOAD_KEY_SENSOR_DATA_TYPE_HUMIDITY[] = "humid";

const char CERT_FORM_DER[] = "der";

const char CERT_TYPE_CRT[] = "crt";
const char CERT_TYPE_KEY[] = "key";

const char MODEL_KEY_ADDR[] = "<a>";
const char MODEL_KEY_PORT[] = "<p>";
const char MODEL_KEY_NAMESPACE[] = "<ns>";
const char MODEL_KEY_GROUP[] = "<g>";
const char MODEL_KEY_ID[] = "<id>";
const char MODEL_KEY_FORM[] = "<form>";
const char MODEL_KEY_TYPE[] = "<type>";

const char HEADER_AUTHORIZATION[] = "Authorization";
const char HEADER_CONTENT_TYPE[] = "Content-Type";
const char HEADER_X_MD5[] = "x-MD5";

const char MIME_TYPE_APP_JSON[] = "application/json";

const char URL_MODEL_UPDATE_FW[] = "https://<a>:<p>/fw/update/";
const char URL_MODEL_UPDATE_FW_NOT_S[] = "http://<a>:<p>/fw/update/<id>/";
const char URL_MODEL_FINGERPRINTS[] = "https://<a>:<p>/cert/fingerprints/<a>/";
const char URL_MODEL_FINGERPRINTS_NOT_S[] = "http://<a>:<p>/cert/fingerprints/<a>/";
const char URL_MODEL_TOKEN[] = "https://<a>:<p>/auth/token/";
const char URL_MODEL_CERT_CA[] = "https://<a>:<p>/cert/ca/<form>/";
const char URL_MODEL_CERT[] = "https://<a>:<p>/cert/client/<type>/<form>/";

const char TOPIC_MODEL_CONFIG[] = "<ns>/<g>/<id>/config";
const char TOPIC_MODEL_DATA[] = "<ns>/<g>/<id>/data";

const char FILE_NAME_TEMP_DOWNLOAD[] = "/download.tmp";
const char FILE_NAME_CERT_CA_CRT[] = "/ca.crt";
const char FILE_NAME_CERT_CRT[] = "/crt";
const char FILE_NAME_CERT_KEY[] = "/key";

} // Strings

}}
