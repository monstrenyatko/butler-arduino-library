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

#ifndef BUTLER_ARDUINO_STRINGS_H_
#define BUTLER_ARDUINO_STRINGS_H_

/* System Includes */
/* Internal Includes */


namespace Butler {
namespace Arduino {

namespace Strings {

extern const char EMPTY[];
extern const char SPACE[];
extern const char APP[];
extern const char WIFI[];
extern const char AUTH[];
extern const char PERIOD[];
extern const char SSID[];
extern const char PASSPHRASE[];
extern const char PAIRED[];
extern const char FINGERPRINTS[];
extern const char TOKEN[];
extern const char USERNAME[];
extern const char PASSWORD[];

extern const char PAYLOAD_KEY_VERSION[];
extern const char PAYLOAD_KEY_ID[];
extern const char PAYLOAD_KEY_DATA[];
extern const char PAYLOAD_KEY_RESULTS[];
extern const char PAYLOAD_KEY_VALUE[];
extern const char PAYLOAD_KEY_SENSOR_DATA_TYPE_TEMPERATURE[];
extern const char PAYLOAD_KEY_SENSOR_DATA_TYPE_HUMIDITY[];

extern const char CERT_FORM_DER[];

extern const char CERT_TYPE_CRT[];
extern const char CERT_TYPE_KEY[];

extern const char MODEL_KEY_ADDR[];
extern const char MODEL_KEY_PORT[];
extern const char MODEL_KEY_NAMESPACE[];
extern const char MODEL_KEY_GROUP[];
extern const char MODEL_KEY_ID[];
extern const char MODEL_KEY_FORM[];
extern const char MODEL_KEY_TYPE[];

extern const char HEADER_AUTHORIZATION[];
extern const char HEADER_CONTENT_TYPE[];
extern const char HEADER_X_MD5[];

extern const char MIME_TYPE_APP_JSON[];

extern const char URL_MODEL_UPDATE_FW[];
extern const char URL_MODEL_UPDATE_FW_NOT_S[];
extern const char URL_MODEL_FINGERPRINTS[];
extern const char URL_MODEL_FINGERPRINTS_NOT_S[];
extern const char URL_MODEL_TOKEN[];
extern const char URL_MODEL_CERT_CA[];
extern const char URL_MODEL_CERT[];

extern const char TOPIC_MODEL_CONFIG[];
extern const char TOPIC_MODEL_DATA[];

extern const char FILE_NAME_TEMP_DOWNLOAD[];
extern const char FILE_NAME_CERT_CA_CRT[];
extern const char FILE_NAME_CERT_CRT[];
extern const char FILE_NAME_CERT_KEY[];

} // Strings

}}

#endif // BUTLER_ARDUINO_STRINGS_H_
