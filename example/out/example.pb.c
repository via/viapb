#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"

#include "viapb-private.h"
#include "example.pb.h"

// Types from example.proto

static bool pb_decode_enum_MyEnum(MyEnum *value, pb_read_fn r, void *user) {
  uint32_t temp;
  if (!pb_decode_varint_uint32(&temp, r, user)) { return false; }
  *value = temp;
  return true;
}

unsigned pb_sizeof_SubMessage(const struct SubMessage *msg) {
  unsigned size = 0;
  if (msg->type != 0) {
    size += 1;  // Size of tag
    unsigned element_size = pb_sizeof_varint(msg->type);
    size += element_size;
  }

  if (msg->some_text.len > 0) {
    size += 1;  // Size of tag
    unsigned element_size = msg->some_text.len;
    size += pb_sizeof_varint(element_size);
    size += element_size;
  }


  return size;
}
bool pb_encode_SubMessage(const struct SubMessage *msg, pb_write_fn w, void *user) {
  uint8_t scratch[20];
  if (msg->type != 0) {
    uint8_t *ptr = scratch;
    ptr += pb_encode_varint(ptr, (1 << 3) | PB_WT_VARINT);
    ptr += pb_encode_varint(ptr, msg->type);
    if (!w(scratch, ptr - scratch, user)) { return false; }
  }

  if (msg->some_text.len > 0) {
    uint8_t *ptr = scratch;
    ptr += pb_encode_varint(ptr, (2 << 3) | PB_WT_STRING);
    unsigned elem_size = msg->some_text.len;
    ptr += pb_encode_varint(ptr, elem_size);
    if (!w(scratch, ptr - scratch, user)) { return false; }
    if (!w((const uint8_t *)msg->some_text.str, msg->some_text.len, user)) { return false; }
  }

  return true;
}
unsigned pb_encode_SubMessage_to_buffer(uint8_t buffer[20], const struct SubMessage *msg) {
  uint8_t *ptr = buffer;
  if (msg->type != 0) {
    ptr += pb_encode_varint(ptr, (1 << 3) | PB_WT_VARINT);
    ptr += pb_encode_varint(ptr, msg->type);
  }

  if (msg->some_text.len > 0) {
    ptr += pb_encode_varint(ptr, (2 << 3) | PB_WT_STRING);
    unsigned elem_size = msg->some_text.len;
    ptr += pb_encode_varint(ptr, elem_size);
    memcpy(ptr, msg->some_text.str, msg->some_text.len);
    ptr += msg->some_text.len;
  }

  return (ptr - buffer);
}
static bool pb_decode_enum_SubMessage_NegativeEnum(SubMessage_NegativeEnum *value, pb_read_fn r, void *user) {
  int32_t temp;
  if (!pb_decode_varint_int32(&temp, r, user)) { return false; }
  *value = temp;
  return true;
}

bool pb_decode_SubMessage(struct SubMessage *msg, pb_read_fn r, void *user) {
  while (true) {
    uint32_t prefix;
    if (!pb_decode_varint_uint32(&prefix, r, user)) { break; }
    if (prefix == ((1ul << 3) | PB_WT_VARINT)) {
      if (!pb_decode_enum_MyEnum(&msg->type, r, user)) { return false; }
    }
    if (prefix == ((2ul << 3) | PB_WT_STRING)) {
      uint32_t length;
      if (!pb_decode_varint_uint32(&length, r, user)) { return false; }
      if (length > 16) { return false; }
      if (!r((uint8_t *)msg->some_text.str, length, user)) { return false; }
      msg->some_text.len = length;
    }
  }
  return true;
}
unsigned pb_sizeof_MultiMessage(const struct MultiMessage *msg) {
  unsigned size = 0;
  if (msg->some_int != 0) {
    size += 1;  // Size of tag
    unsigned element_size = pb_sizeof_varint(msg->some_int);
    size += element_size;
  }

  if (msg->which_some_oneof == PB_TAG_MultiMessage_part_one) {
    size += 1;  // Size of tag
    unsigned element_size = pb_sizeof_varint(msg->part_one);
    size += element_size;
  }

  if (msg->which_some_oneof == PB_TAG_MultiMessage_part_two) {
    size += 1;  // Size of tag
    unsigned element_size = msg->part_two.len;
    size += pb_sizeof_varint(element_size);
    size += element_size;
  }


  return size;
}
bool pb_encode_MultiMessage(const struct MultiMessage *msg, pb_write_fn w, void *user) {
  uint8_t scratch[20];
  if (msg->some_int != 0) {
    uint8_t *ptr = scratch;
    ptr += pb_encode_varint(ptr, (1 << 3) | PB_WT_VARINT);
    ptr += pb_encode_varint(ptr, msg->some_int);
    if (!w(scratch, ptr - scratch, user)) { return false; }
  }

  if (msg->which_some_oneof == PB_TAG_MultiMessage_part_one) {
    uint8_t *ptr = scratch;
    ptr += pb_encode_varint(ptr, (2 << 3) | PB_WT_VARINT);
    ptr += pb_encode_varint(ptr, msg->part_one);
    if (!w(scratch, ptr - scratch, user)) { return false; }
  }

  if (msg->which_some_oneof == PB_TAG_MultiMessage_part_two) {
    uint8_t *ptr = scratch;
    ptr += pb_encode_varint(ptr, (3 << 3) | PB_WT_STRING);
    unsigned elem_size = msg->part_two.len;
    ptr += pb_encode_varint(ptr, elem_size);
    if (!w(scratch, ptr - scratch, user)) { return false; }
    if (!w((const uint8_t *)msg->part_two.str, msg->part_two.len, user)) { return false; }
  }

  return true;
}
unsigned pb_encode_MultiMessage_to_buffer(uint8_t buffer[29], const struct MultiMessage *msg) {
  uint8_t *ptr = buffer;
  if (msg->some_int != 0) {
    ptr += pb_encode_varint(ptr, (1 << 3) | PB_WT_VARINT);
    ptr += pb_encode_varint(ptr, msg->some_int);
  }

  if (msg->which_some_oneof == PB_TAG_MultiMessage_part_one) {
    ptr += pb_encode_varint(ptr, (2 << 3) | PB_WT_VARINT);
    ptr += pb_encode_varint(ptr, msg->part_one);
  }

  if (msg->which_some_oneof == PB_TAG_MultiMessage_part_two) {
    ptr += pb_encode_varint(ptr, (3 << 3) | PB_WT_STRING);
    unsigned elem_size = msg->part_two.len;
    ptr += pb_encode_varint(ptr, elem_size);
    memcpy(ptr, msg->part_two.str, msg->part_two.len);
    ptr += msg->part_two.len;
  }

  return (ptr - buffer);
}
bool pb_decode_MultiMessage(struct MultiMessage *msg, pb_read_fn r, void *user) {
  while (true) {
    uint32_t prefix;
    if (!pb_decode_varint_uint32(&prefix, r, user)) { break; }
    if (prefix == ((1ul << 3) | PB_WT_VARINT)) {
      if (!pb_decode_varint_int32(&msg->some_int, r, user)) { return false; }
    }
    if (prefix == ((2ul << 3) | PB_WT_VARINT)) {
      if (!pb_decode_varint_int32(&msg->part_one, r, user)) { return false; }
      msg->which_some_oneof = 2;
    }
    if (prefix == ((3ul << 3) | PB_WT_STRING)) {
      uint32_t length;
      if (!pb_decode_varint_uint32(&length, r, user)) { return false; }
      if (length > 16) { return false; }
      if (!r((uint8_t *)msg->part_two.str, length, user)) { return false; }
      msg->part_two.len = length;
      msg->which_some_oneof = 3;
    }
  }
  return true;
}
unsigned pb_sizeof_MyMessage(const struct MyMessage *msg) {
  unsigned size = 0;
  if (msg->some_integer != 0) {
    size += 1;  // Size of tag
    unsigned element_size = pb_sizeof_varint(msg->some_integer);
    size += element_size;
  }

  if (msg->some_fixed != 0) {
    size += 1;  // Size of tag
    unsigned element_size = 4;
    size += element_size;
  }

  if (msg->has_some_submsg) {
    size += 1;  // Size of tag
    unsigned element_size = pb_sizeof_SubMessage(&msg->some_submsg);
    size += pb_sizeof_varint(element_size);
    size += element_size;
  }

  if (msg->many_integers_count > 0) {
    unsigned packed_size = 0;
    for (unsigned i = 0; i < msg->many_integers_count; i++) {
      packed_size += pb_sizeof_varint(msg->many_integers[i]);
    }
    size += 1;  // Size of tag
    size += pb_sizeof_varint(packed_size);
    size += packed_size;
  }

  if (msg->many_fixed_count > 0) {
    unsigned packed_size = msg->many_fixed_count * 4;
    size += 1;  // Size of tag
    size += pb_sizeof_varint(packed_size);
    size += packed_size;
  }

  if (msg->many_submsg_count > 0) {
    for (unsigned i = 0; i < msg->many_submsg_count; i++) {
      size += 1;  // Size of tag
      unsigned element_size = pb_sizeof_SubMessage(&msg->many_submsg[i]);
      size += pb_sizeof_varint(element_size);
      size += element_size;
    }
  }


  return size;
}
bool pb_encode_MyMessage(const struct MyMessage *msg, pb_write_fn w, void *user) {
  uint8_t scratch[20];
  if (msg->some_integer != 0) {
    uint8_t *ptr = scratch;
    ptr += pb_encode_varint(ptr, (1 << 3) | PB_WT_VARINT);
    ptr += pb_encode_varint(ptr, msg->some_integer);
    if (!w(scratch, ptr - scratch, user)) { return false; }
  }

  if (msg->some_fixed != 0) {
    uint8_t *ptr = scratch;
    ptr += pb_encode_varint(ptr, (2 << 3) | PB_WT_32BIT);
    ptr += pb_encode_fixed32(ptr, msg->some_fixed);
    if (!w(scratch, ptr - scratch, user)) { return false; }
  }

  if (msg->has_some_submsg) {
    uint8_t *ptr = scratch;
    ptr += pb_encode_varint(ptr, (3 << 3) | PB_WT_STRING);
    unsigned elem_size = pb_sizeof_SubMessage(&msg->some_submsg);
    ptr += pb_encode_varint(ptr, elem_size);
    if (!w(scratch, ptr - scratch, user)) { return false; }
    if (!pb_encode_SubMessage(&msg->some_submsg, w, user)) { return false; }
  }

  if (msg->many_integers_count > 0) {
    uint8_t *ptr = scratch;
    ptr += pb_encode_varint(ptr, (4 << 3) | PB_WT_STRING);
    unsigned sz = 0;
    for (unsigned idx = 0; idx < msg->many_integers_count; idx++) {
      sz += pb_sizeof_varint(msg->many_integers[idx]);
    }
    ptr += pb_encode_varint(ptr, sz);
    if (!w(scratch, ptr - scratch, user)) { return false; }
    for (unsigned idx = 0; idx < msg->many_integers_count; idx++) {
      unsigned len = pb_encode_varint(scratch, msg->many_integers[idx]);
      if (!w(scratch, len, user)) { return false; }
    }
  }

  if (msg->many_fixed_count > 0) {
    uint8_t *ptr = scratch;
    ptr += pb_encode_varint(ptr, (5 << 3) | PB_WT_STRING);
    unsigned sz = 4 * msg->many_fixed_count;
    ptr += pb_encode_varint(ptr, sz);
    if (!w(scratch, ptr - scratch, user)) { return false; }
    for (unsigned idx = 0; idx < msg->many_fixed_count; idx++) {
      unsigned len = pb_encode_fixed32(scratch, msg->many_fixed[idx]);
      if (!w(scratch, len, user)) { return false; }
    }
  }

  if (msg->many_submsg_count > 0) {
    for (unsigned idx = 0; idx < msg->many_submsg_count; idx++) {
      uint8_t *ptr = scratch;
      ptr += pb_encode_varint(ptr, (6 << 3) | PB_WT_STRING);
      unsigned elem_size = pb_sizeof_SubMessage(&msg->many_submsg[idx]);
      ptr += pb_encode_varint(ptr, elem_size);
      if (!w(scratch, ptr - scratch, user)) { return false; }
      if (!pb_encode_SubMessage(&msg->many_submsg[idx], w, user)) { return false; }
    }
  }

  return true;
}
unsigned pb_encode_MyMessage_to_buffer(uint8_t buffer[186], const struct MyMessage *msg) {
  uint8_t *ptr = buffer;
  if (msg->some_integer != 0) {
    ptr += pb_encode_varint(ptr, (1 << 3) | PB_WT_VARINT);
    ptr += pb_encode_varint(ptr, msg->some_integer);
  }

  if (msg->some_fixed != 0) {
    ptr += pb_encode_varint(ptr, (2 << 3) | PB_WT_32BIT);
    ptr += pb_encode_fixed32(ptr, msg->some_fixed);
  }

  if (msg->has_some_submsg) {
    ptr += pb_encode_varint(ptr, (3 << 3) | PB_WT_STRING);
    unsigned elem_size = pb_sizeof_SubMessage(&msg->some_submsg);
    ptr += pb_encode_varint(ptr, elem_size);
    ptr += pb_encode_SubMessage_to_buffer(ptr, &msg->some_submsg);
  }

  if (msg->many_integers_count > 0) {
    ptr += pb_encode_varint(ptr, (4 << 3) | PB_WT_STRING);
    unsigned sz = 0;
    for (unsigned idx = 0; idx < msg->many_integers_count; idx++) {
      sz += pb_sizeof_varint(msg->many_integers[idx]);
    }
    ptr += pb_encode_varint(ptr, sz);
    for (unsigned idx = 0; idx < msg->many_integers_count; idx++) {
      ptr += pb_encode_varint(ptr, msg->many_integers[idx]);
    }
  }

  if (msg->many_fixed_count > 0) {
    ptr += pb_encode_varint(ptr, (5 << 3) | PB_WT_STRING);
    unsigned sz = 4 * msg->many_fixed_count;
    ptr += pb_encode_varint(ptr, sz);
    for (unsigned idx = 0; idx < msg->many_fixed_count; idx++) {
      ptr += pb_encode_fixed32(ptr, msg->many_fixed[idx]);
    }
  }

  if (msg->many_submsg_count > 0) {
    for (unsigned idx = 0; idx < msg->many_submsg_count; idx++) {
      ptr += pb_encode_varint(ptr, (6 << 3) | PB_WT_STRING);
      unsigned elem_size = pb_sizeof_SubMessage(&msg->many_submsg[idx]);
      ptr += pb_encode_varint(ptr, elem_size);
      ptr += pb_encode_SubMessage_to_buffer(ptr, &msg->many_submsg[idx]);
    }
  }

  return (ptr - buffer);
}
bool pb_decode_MyMessage(struct MyMessage *msg, pb_read_fn r, void *user) {
  while (true) {
    uint32_t prefix;
    if (!pb_decode_varint_uint32(&prefix, r, user)) { break; }
    if (prefix == ((1ul << 3) | PB_WT_VARINT)) {
      if (!pb_decode_varint_int32(&msg->some_integer, r, user)) { return false; }
    }
    if (prefix == ((2ul << 3) | PB_WT_32BIT)) {
      if (!pb_decode_fixed32(&msg->some_fixed, r, user)) { return false; }
    }
    if (prefix == ((3ul << 3) | PB_WT_STRING)) {
      uint32_t length;
      if (!pb_decode_varint_uint32(&length, r, user)) { return false; }
      struct pb_bounded_reader br = { .r = r, .user = user, .len = length };
      if (!pb_decode_SubMessage(&msg->some_submsg, pb_bounded_read, &br)) { return false; }
      if (br.len != 0) return false;
      msg->has_some_submsg = true;
    }
    if (prefix == ((4ul << 3) | PB_WT_STRING)) {
      uint32_t length;
      if (!pb_decode_varint_uint32(&length, r, user)) { return false; }
      struct pb_bounded_reader br = { .r = r, .user = user, .len = length };
      while (br.len  > 0) {
        if (msg->many_integers_count >= 4) { return false; }
        if (!pb_decode_varint_int32(&msg->many_integers[msg->many_integers_count], pb_bounded_read, &br)) { return false; }
        msg->many_integers_count++;
      }
      if (br.len != 0) return false;
    }
    if (prefix == ((4ul << 3) | PB_WT_VARINT)) {
      if (msg->many_integers_count >= 4) { return false; }
      if (!pb_decode_varint_int32(&msg->many_integers[msg->many_integers_count], r, user)) { return false; }
      msg->many_integers_count++;
    }
    if (prefix == ((5ul << 3) | PB_WT_STRING)) {
      uint32_t length;
      if (!pb_decode_varint_uint32(&length, r, user)) { return false; }
      struct pb_bounded_reader br = { .r = r, .user = user, .len = length };
      while (br.len  > 0) {
        if (msg->many_fixed_count >= 4) { return false; }
        if (!pb_decode_fixed32(&msg->many_fixed[msg->many_fixed_count], pb_bounded_read, &br)) { return false; }
        msg->many_fixed_count++;
      }
      if (br.len != 0) return false;
    }
    if (prefix == ((5ul << 3) | PB_WT_32BIT)) {
      if (msg->many_fixed_count >= 4) { return false; }
      if (!pb_decode_fixed32(&msg->many_fixed[msg->many_fixed_count], r, user)) { return false; }
      msg->many_fixed_count++;
    }
    if (prefix == ((6ul << 3) | PB_WT_STRING)) {
      uint32_t length;
      if (!pb_decode_varint_uint32(&length, r, user)) { return false; }
      if (length > 20) { return false; }
      if (msg->many_submsg_count >= 4) { return false; }
      struct pb_bounded_reader br = { .r = r, .user = user, .len = length };
      if (!pb_decode_SubMessage(&msg->many_submsg[msg->many_submsg_count], pb_bounded_read, &br)) { return false; }
      if (br.len != 0) return false;
      msg->many_submsg_count++;
    }
  }
  return true;
}

