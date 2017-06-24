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
#include <WString.h>
/* Internal Includes */


namespace Butler {
namespace Arduino {

namespace Strings {

const char EMPTY[] = "";
const char WIFI[] = "wifi";
const char PERIOD[] = "period";
const char SSID[] = "ssid";
const char PASSPHRASE[] = "passphrase";

} // Strings

}}

#endif // BUTLER_ARDUINO_STRINGS_H_
