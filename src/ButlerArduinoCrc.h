/*
 *******************************************************************************
 *
 * Purpose: CRC implementation.
 *
 *******************************************************************************
 * Copyright Oleg Kovalenko 2017.
 *
 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

#ifndef BUTLER_ARDUINO_CRC_H_
#define BUTLER_ARDUINO_CRC_H_

/* System Includes */
#include <stdint.h>
/* Internal Includes */


namespace Butler {
namespace Arduino {

namespace Crc {

uint32_t crc32Begin();

uint32_t crc32Continue(uint32_t crc, const uint8_t b[], uint32_t len);

uint32_t crc32End(uint32_t crc);

uint32_t crc32(const char *s);

uint32_t crc32(const uint8_t b[], uint32_t len);

} // Crc

}}

#endif // BUTLER_ARDUINO_CRC_H_

