#ifndef VIAPB_OUT_EXAMPLE_PB_H
#define VIAPB_OUT_EXAMPLE_PB_H

#include <stdint.h>
#include <stdbool.h>

#include "viapb.h"

// Types from example.proto

typedef enum {
  MyEnum_SomeValue = 0,
  MyEnum_AnotherValue = 1,
} MyEnum;


// Nested enums for SubMessage

typedef enum {
  SubMessage_NegativeEnum_SomeValue = 0,
  SubMessage_NegativeEnum_LowValue = -2,
  SubMessage_NegativeEnum_HighValue = 65535,
} SubMessage_NegativeEnum;


struct SubMessage {
  MyEnum type;

  struct { char str[16]; unsigned len; } some_text;

};

bool pb_encode_SubMessage(const struct SubMessage *msg, pb_write_fn w, void *user);
unsigned pb_encode_SubMessage_to_buffer(uint8_t buffer[20], const struct SubMessage *msg);
unsigned pb_sizeof_SubMessage(const struct SubMessage *msg);

bool pb_decode_SubMessage(struct SubMessage *msg, pb_read_fn r, void *user);
#define PB_TAG_SubMessage_type     1ul
#define PB_TAG_SubMessage_some_text     2ul
#define PB_MAX_SIZE_SubMessage    20ul


struct MultiMessage {
  int32_t some_int;

  unsigned which_some_oneof;
  union {
    int32_t part_one;
    struct { char str[16]; unsigned len; } part_two;
  } some_oneof;

};

bool pb_encode_MultiMessage(const struct MultiMessage *msg, pb_write_fn w, void *user);
unsigned pb_encode_MultiMessage_to_buffer(uint8_t buffer[29], const struct MultiMessage *msg);
unsigned pb_sizeof_MultiMessage(const struct MultiMessage *msg);

bool pb_decode_MultiMessage(struct MultiMessage *msg, pb_read_fn r, void *user);
#define PB_TAG_MultiMessage_some_int     1ul
#define PB_TAG_MultiMessage_part_one     2ul
#define PB_TAG_MultiMessage_part_two     3ul
#define PB_MAX_SIZE_MultiMessage    29ul


struct MyMessage {
  int32_t some_integer;

  uint32_t some_fixed;

  bool has_some_submsg;
  struct SubMessage some_submsg;

  unsigned many_integers_count;
  int32_t many_integers[4];

  unsigned many_fixed_count;
  uint32_t many_fixed[4];

  unsigned many_submsg_count;
  struct SubMessage many_submsg[4];

};

bool pb_encode_MyMessage(const struct MyMessage *msg, pb_write_fn w, void *user);
unsigned pb_encode_MyMessage_to_buffer(uint8_t buffer[186], const struct MyMessage *msg);
unsigned pb_sizeof_MyMessage(const struct MyMessage *msg);

bool pb_decode_MyMessage(struct MyMessage *msg, pb_read_fn r, void *user);
#define PB_TAG_MyMessage_some_integer     1ul
#define PB_TAG_MyMessage_some_fixed     2ul
#define PB_TAG_MyMessage_some_submsg     3ul
#define PB_TAG_MyMessage_many_integers     4ul
#define PB_TAG_MyMessage_many_fixed     5ul
#define PB_TAG_MyMessage_many_submsg     6ul
#define PB_MAX_SIZE_MyMessage    186ul


#endif // VIAPB_OUT_EXAMPLE_PB_H


