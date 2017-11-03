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

#ifndef BUTLER_ARDUINO_MD5_H_
#define BUTLER_ARDUINO_MD5_H_

/* System Includes */
#include <WString.h>
#include <FS.h>
/* Internal Includes */


namespace Butler {
namespace Arduino {

namespace Md5 {

/**
 * Calculates the file MD5.
 *
 * Returns empty string in case of error.
 */
String md5(File &file);

} // Md5

}}

#endif // BUTLER_ARDUINO_MD5_H_

