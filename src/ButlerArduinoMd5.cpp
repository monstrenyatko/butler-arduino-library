/*
 *******************************************************************************
 *
 * Purpose: MD5 implementation.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifdef ESP8266

/* System Includes */
#include <MD5Builder.h>
/* Internal Includes */
#include "ButlerArduinoMd5.h"


namespace Butler {
namespace Arduino {

namespace Md5 {

String md5(File &file) {
	if (!file) {
		return String();
	}
	if (file.seek(0, SeekSet)) {
		MD5Builder md5;
		md5.begin();
		md5.addStream(file, file.size());
		md5.calculate();
		return md5.toString();
	} else {
		return String();
	}
}

} // Md5

}}

#endif // ESP8266
