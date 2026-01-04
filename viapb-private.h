#ifndef VIAPB_PRIVATE_H
#define VIAPB_PRIVATE_H

/* Private header for viaPB helper functions.
 * This is intended to only be included by generated source files 
 *
 * A few functions:
 *  - pb_zigzag
 *  - pb_encode_varint
 *  - pb_(en|de)code_fixed(32|64)
 * while modified, are derived from nanopb, written by Petteri Aimonen. 
 * These functions were licensed in nanopb under the zlib license,
 * the text of which is reproduced here:
 *
 * Copyright (c) 2011 Petteri Aimonen <jpa at nanopb.mail.kapsi.fi>
 * 
 * This software is provided 'as-is', without any express or 
 * implied warranty. In no event will the authors be held liable 
 * for any damages arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any 
 * purpose, including commercial applications, and to alter it and 
 * redistribute it freely, subject to the following restrictions:
 * 
 * 1. The origin of this software must not be misrepresented; you 
 *    must not claim that you wrote the original software. If you use 
 *    this software in a product, an acknowledgment in the product 
 *    documentation would be appreciated but is not required.
 * 
 * 2. Altered source versions must be plainly marked as such, and 
 *    must not be misrepresented as being the original software.
 * 
 * 3. This notice may not be removed or altered from any source 
 *    distribution.
 *
 */

#include <stdint.h>
#include <stdbool.h>

#include "viapb.h"

#pragma GCC diagnostic ignored "-Wunused-function"

static unsigned pb_sizeof_varint(uint64_t value) {
  if (value < (1ull << (7 * 1)))
    return 1;
  if (value < (1ull << (7 * 2)))
    return 2;
  if (value < (1ull << (7 * 3)))
    return 3;
  if (value < (1ull << (7 * 4)))
    return 4;
  if (value < (1ull << (7 * 5)))
    return 5;
  if (value < (1ull << (7 * 6)))
    return 6;
  if (value < (1ull << (7 * 7)))
    return 7;
  if (value < (1ull << (7 * 8)))
    return 8;
  if (value < (1ull << (7 * 9)))
    return 9;

  return 10;
}

static uint64_t pb_zigzag(int64_t value) {
  uint64_t uvalue = value >= 0 ? 
    ((uint64_t)value * 2) : 
    ((uint64_t)-value * 2 + 1);
  return uvalue;
}

static int64_t pb_unzigzag(uint64_t value) {
  int64_t ivalue = value >> 1;
  if (value & 0x1) {
    ivalue = -ivalue;
  }
  return ivalue;
}

static unsigned pb_sizeof_svarint(int64_t value) {
  return pb_sizeof_varint(pb_zigzag(value));
}

static unsigned pb_encode_varint(uint8_t *buf, uint64_t value) {
  if (value <= 0x7f) {
    buf[0] = (uint8_t)value;
    return 1;
  }

  unsigned i = 0;
  uint32_t low = value & 0xffffffff;
  uint32_t high = value >> 32;
  uint8_t byte = low & 0x7f;

  low >>= 7;
  while (i < 4 && (low != 0 || high != 0)) {
    byte |= 0x80;
    buf[i++] = byte;
    byte = low & 0x7f;
    low >>= 7;
  }

  if (high != 0) {
    byte = byte | ((high & 0x07) << 4);
    high >>= 3;

    while (high > 0) {
      byte |= 0x80;
      buf[i++] = byte;
      byte = (uint8_t)(high & 0x7F);
      high >>= 7;
    }
  }

  buf[i++] = byte;
  return i;
}

static unsigned pb_encode_svarint(uint8_t dest[10], int64_t value) {
  return pb_encode_varint(dest, pb_zigzag(value));
}


/* Use bitshifts and masks to serialize uint32s and uint64s as little endian.  The intermediate buffer is used with a memcpy, as doing it directly to
 * dest did not reliably produce a single (unaligned) store */

static unsigned pb_encode_fixed32(uint8_t dest[4], const uint32_t value) {
  uint8_t buf[4];
  buf[0] = value & 0xff;
  buf[1] = (value >> 8) & 0xff;
  buf[2] = (value >> 16) & 0xff;
  buf[3] = (value >> 24) & 0xff;

  memcpy(dest, buf, 4);
  return 4;
}

static unsigned pb_encode_fixed64(uint8_t dest[8], const uint64_t value) {
  uint8_t buf[8];
  buf[0] = value & 0xff;
  buf[1] = (value >> 8) & 0xff;
  buf[2] = (value >> 16) & 0xff;
  buf[3] = (value >> 24) & 0xff;
  buf[4] = (value >> 32) & 0xff;
  buf[5] = (value >> 40) & 0xff;
  buf[6] = (value >> 48) & 0xff;
  buf[7] = (value >> 56) & 0xff;
  memcpy(dest, buf, 8);
  return 8;
}

static unsigned pb_encode_float(uint8_t buf[4], const float value) {
  memcpy(buf, &value, 4);
  return 4;
}

static unsigned pb_encode_double(uint8_t buf[8], const double value) {
  memcpy(buf, &value, 8);
  return 4;
}

static bool pb_decode_varint_uint64(uint64_t *value, pb_read_fn r, void *user) {
  uint8_t bytes[10];
  unsigned int idx = 0;
  do {
    if (!r(&bytes[idx], 1, user)) { 
      return false; 
    }
    if ((bytes[idx] & 0x80) == 0) {
      break;
    }
    idx++;
  } while (idx < 10);

  if (idx == 10) {
    return false;
  }


  uint64_t result = 0;
  while (idx > 0) {
    result = (result << 7) | (bytes[idx] & 0x7f);
    idx--;
  }
  result = (result << 7) | (bytes[0] & 0x7f);
  *value = result;
  return true;
}

static bool pb_decode_varint_int64(int64_t *value, pb_read_fn r, void *user) {
  uint64_t uvalue;
  if (!pb_decode_varint_uint64(&uvalue, r, user)) {
    return false;
  }
  *value = uvalue;
  return true;
}

static bool pb_decode_svarint_int64(int64_t *value, pb_read_fn r, void *user) {
  uint64_t uvalue;
  if (!pb_decode_varint_uint64(&uvalue, r, user)) {
    return false;
  }
  *value = pb_unzigzag(uvalue);
  return true;
}

static bool pb_decode_varint_uint32(uint32_t *value, pb_read_fn r, void *user) {
  uint8_t bytes[5];
  if (!r(&bytes[0], 1, user)) {
    return false;
  }
  if ((bytes[0] & 0x80) == 0) {
    *value = bytes[0];
    return true;
  }

  unsigned int idx = 1;
  do {
    if (!r(&bytes[idx], 1, user)) { 
      return false; 
    }
    if ((bytes[idx] & 0x80) == 0) {
      break;
    }
    idx++;
  } while (idx < 5);

  if (idx == 5) {
    return false;
  }


  uint64_t result = 0;
  while (idx > 0) {
    result = (result << 7) | (bytes[idx] & 0x7f);
    idx--;
  }
  result = (result << 7) | (bytes[0] & 0x7f);
  *value = result;
  return true;
}

static bool pb_decode_varint_int32(int32_t *value, pb_read_fn r, void *user) {
  uint64_t uvalue;
  if (!pb_decode_varint_uint64(&uvalue, r, user)) {
    return false;
  }
  uint32_t high = uvalue >> 32;
  if ((high == 0) || (high == 0xFFFFFFFF)) {
    *value = uvalue;
    return true;
  } else {
    return false;
  }
}

static bool pb_decode_svarint_int32(int32_t *value, pb_read_fn r, void *user) {
  uint64_t uvalue;
  if (!pb_decode_varint_uint64(&uvalue, r, user)) {
    return false;
  }
  *value = pb_unzigzag(uvalue);
  return true;
}

static bool pb_decode_fixed32(uint32_t *value, pb_read_fn r, void *user) { 
  uint8_t bytes[4];
  if (!r(bytes, 4, user)) {
    return false;
  }
  *value = ((uint32_t)bytes[3] << 24) |
           ((uint32_t)bytes[2] << 16) |
           ((uint32_t)bytes[1] << 8)  |
           (uint32_t)bytes[0];
  return true;
}

static bool pb_decode_fixed64(uint64_t *value, pb_read_fn r, void *user) { 
  uint8_t bytes[8];
  if (!r(bytes, 8, user)) {
    return false;
  }
  *value = ((uint64_t)bytes[7] << 56) |
           ((uint64_t)bytes[6] << 48) |
           ((uint64_t)bytes[5] << 40) |
           ((uint64_t)bytes[4] << 32) |
           ((uint64_t)bytes[3] << 24) |
           ((uint64_t)bytes[2] << 16) |
           ((uint64_t)bytes[1] << 8)  |
           (uint64_t)bytes[0];
  return true;
}

static bool pb_decode_sfixed32(int32_t *value, pb_read_fn r, void *user) { 
  uint32_t uvalue;
  if (!pb_decode_fixed32(&uvalue, r, user)) { return false; }
  *value = uvalue;
  return true;
}

static bool pb_decode_sfixed64(int64_t *value, pb_read_fn r, void *user) { 
  uint64_t uvalue;
  if (!pb_decode_fixed64(&uvalue, r, user)) { return false; }
  *value = uvalue;
  return true;
}

static bool pb_decode_bool(bool *value, pb_read_fn r, void *user) {
  uint8_t byte;
  if (!r(&byte, 1, user)) {
    return false;
  }
  if (byte > 1) {
    return false;
  }
  *value = byte;
  return true;
}


static bool pb_decode_float(float *value, pb_read_fn r, void *user) { 
  uint8_t bytes[4];
  if (!r(bytes, 4, user)) {
    return false;
  }
  memcpy(value, bytes, 4);
  return true;
}

static bool pb_decode_double(double *value, pb_read_fn r, void *user) { 
  uint8_t bytes[8];
  if (!r(bytes, 8, user)) {
    return false;
  }
  memcpy(value, bytes, 8);
  return true;
}

struct pb_bounded_reader {
  pb_read_fn r;
  void *user;
  unsigned len;
};


static bool pb_bounded_read(uint8_t *data, unsigned len, void *user) {
  struct pb_bounded_reader *b = (struct pb_bounded_reader *)user;
  if (len > b->len) {
    return false;
  }
  if (!b->r(data, len, b->user)) { return false; }
  b->len -= len;
  return true;
}


#define PB_WT_VARINT 0
#define PB_WT_64BIT  1
#define PB_WT_STRING 2
#define PB_WT_32BIT  5

#endif
