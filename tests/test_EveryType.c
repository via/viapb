#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "EveryType.pb.h"
#include "out/EveryType.pb.h"

static bool file_write(const uint8_t *data, unsigned length, void *ptr) {
  FILE *f = (FILE *)ptr;
  if (fwrite(data, 1, length, f) != length) {
    return false;
  }
  return true;
}

static bool file_read(uint8_t *data, unsigned length, void *ptr) {
  FILE *f = (FILE *)ptr;
  if (fread(data, 1, length, f) != length) {
    return false;
  }
  return true;
}

struct AllTypes build_message(int testcase) {
  (void)testcase;
  struct AllTypes result = { 0 };
  if (testcase == 1) {
    /* testcase 1 is all fields populated with something, basic sanity check */
    result.one_int32 = 1;
    result.one_uint32 = 2;
    result.one_sint32 = 3;
    result.one_int64 = 4;
    result.one_uint64 = 5;
    result.one_sint64 = 6;
    result.one_fixed32 = 7;
    result.one_fixed64 = 8;
    result.one_bool = true;

    const char *s1 = "First String";
    memcpy(result.one_str.str, s1, strlen(s1));
    result.one_str.len = strlen(s1);

    const char *s2 = "First Bytes";
    memcpy(result.one_bytes.bytes, s2, strlen(s2));
    result.one_bytes.len = strlen(s2);

    result.one_float = 9.0;
    result.one_double = 10.0;

    result.one_sfixed32 = 11;
    result.one_sfixed64 = 12;

    result.many_int32[0] = 100;
    result.many_int32[1] = 101;
    result.many_int32[2] = 102;
    result.many_int32[3] = 103;
    result.many_int32_count = 4;

    result.many_uint32[0] = 104;
    result.many_uint32[1] = 105;
    result.many_uint32[2] = 106;
    result.many_uint32[3] = 107;
    result.many_uint32_count = 4;

    result.many_sint32[0] = 108;
    result.many_sint32[1] = 109;
    result.many_sint32[2] = 110;
    result.many_sint32[3] = 111;
    result.many_sint32_count = 4;

    result.many_int64[0] = 112;
    result.many_int64[1] = 113;
    result.many_int64[2] = 114;
    result.many_int64[3] = 115;
    result.many_int64_count = 4;

    result.many_uint64[0] = 116;
    result.many_uint64[1] = 117;
    result.many_uint64[2] = 118;
    result.many_uint64[3] = 119;
    result.many_uint64_count = 4;

    result.many_sint64[0] = 120;
    result.many_sint64[1] = 121;
    result.many_sint64[2] = 122;
    result.many_sint64[3] = 123;
    result.many_sint64_count = 4;

    result.many_fixed32[0] = 124;
    result.many_fixed32[1] = 125;
    result.many_fixed32[2] = 126;
    result.many_fixed32[3] = 127;
    result.many_fixed32_count = 4;

    result.many_fixed64[0] = 128;
    result.many_fixed64[1] = 129;
    result.many_fixed64[2] = 130;
    result.many_fixed64[3] = 131;
    result.many_fixed64_count = 4;

    result.many_bool[0] = true;
    result.many_bool[1] = false;
    result.many_bool[2] = true;
    result.many_bool[3] = false;
    result.many_bool_count = 4;

    for (int i = 0; i < 4; i++) {
      char buf[32];
      sprintf(buf, "String %d", i);
      size_t len = strlen(buf);
      memcpy(result.many_str[i].str, buf, len);
      result.many_str[i].len = len;
    }
    result.many_str_count = 4;

    for (int i = 0; i < 4; i++) {
      char buf[32];
      sprintf(buf, "Bytes %d", i);
      size_t len = strlen(buf);
      memcpy(result.many_bytes[i].bytes, buf, len);
      result.many_bytes[i].len = len;
    }
    result.many_bytes_count = 4;

    result.many_float[0] = 132.0;
    result.many_float[1] = 133.0;
    result.many_float[2] = 134.0;
    result.many_float[3] = 135.0;
    result.many_float_count = 4;

    result.many_double[0] = 136.0;
    result.many_double[1] = 137.0;
    result.many_double[2] = 138.0;
    result.many_double[3] = 139.0;
    result.many_double_count = 4;

    result.many_sfixed32[0] = 140;
    result.many_sfixed32[1] = 141;
    result.many_sfixed32[2] = 142;
    result.many_sfixed32[3] = 143;
    result.many_sfixed32_count = 4;

    result.many_sfixed64[0] = 144;
    result.many_sfixed64[1] = 145;
    result.many_sfixed64[2] = 146;
    result.many_sfixed64[3] = 147;
    result.many_sfixed64_count = 4;

    result.which_selector = PB_TAG_AllTypes_selected_str;
    memcpy(result.selected_str.str, "OneOf", 5); 
    result.selected_str.len = 5;

    result.has_one_msg = true;
    result.one_msg.some_int = 1;
    result.one_msg.has_many_ints = true;
    result.one_msg.many_ints.nested_int_count = 4;
    result.one_msg.many_ints.nested_int[0] = 10;
    result.one_msg.many_ints.nested_int[1] = 11;
    result.one_msg.many_ints.nested_int[2] = INT32_MIN;
    result.one_msg.many_ints.nested_int[3] = 13;
    result.one_msg.e_val = ParentMessage_NestedEnum_More;

    result.many_msg_count = 4;
    result.many_msg[0].some_int = 5;
    result.many_msg[1].some_int = 6;
    result.many_msg[2].some_int = 7;
    result.many_msg[3].some_int = 8;

    result.reg_enum = RegularEnum_One;
    result.larger_enum = LargerEnum_TwoFiveSix;
    result.neg_enum = NegativeEnum_Minus14;

  }
  if (testcase == 2) {
    /* testcase 2, populate scalar varint fields with a worst case size value and negatives */
    result.one_int32 = INT32_MIN;
    result.one_uint32 = UINT32_MAX;
    result.one_sint32 = INT32_MAX;
    result.one_int64 = INT64_MIN;
    result.one_uint64 = UINT64_MAX;
    result.one_sint64 = INT64_MAX;
    result.one_fixed32 = UINT32_MAX;
    result.one_fixed64 = UINT64_MAX;
    result.one_sfixed32 = INT32_MAX;
    result.one_sfixed64 = INT64_MAX;

    result.has_one_msg = true;
    result.one_msg.some_int = INT32_MIN;
    result.larger_enum = LargerEnum_SixteenThousand;
  }
  return result;
}

#define COMPARE(X) if ((a->X) != (b->X)) { fprintf(stderr, "Field mismatch: %s\n", #X); return false; }
#define COMPARE_STR(X) if (((a->X.len) != (b->X.len)) || (memcmp(a->X.str, b->X.str, a->X.len) != 0)) { fprintf(stderr, "Field mismatch: %s\n", #X); return false; }
#define COMPARE_BYTES(X) if (((a->X.len) != (b->X.len)) || (memcmp(a->X.bytes, b->X.bytes, a->X.len) != 0)) { fprintf(stderr, "Field mismatch: %s\n", #X); return false; }

static bool compare_two_parentmsgs(const struct ParentMessage *a, const struct ParentMessage *b) {
  COMPARE(some_int)
  COMPARE(has_many_ints)
  if (a->has_many_ints) {
    COMPARE(many_ints.nested_int_count)
    for (unsigned i = 0; i < a->many_ints.nested_int_count; i++) COMPARE(many_ints.nested_int[i])
  }
  COMPARE(e_val)
  return true;
}

static bool compare_two_alltypes(const struct AllTypes *a, const struct AllTypes *b) {
  COMPARE(one_int32)
  COMPARE(one_uint32)
  COMPARE(one_sint32)
  COMPARE(one_int64)
  COMPARE(one_uint64)
  COMPARE(one_sint64)
  COMPARE(one_fixed32)
  COMPARE(one_fixed64)
  COMPARE(one_bool)
  COMPARE_STR(one_str)
  COMPARE_BYTES(one_bytes)
  COMPARE(one_float)
  COMPARE(one_double)
  COMPARE(one_sfixed32)
  COMPARE(one_sfixed64)

  COMPARE(many_int32_count)
  for (unsigned i = 0; i < a->many_int32_count; i++) COMPARE(many_int32[i]);

  COMPARE(many_uint32_count)
  for (unsigned i = 0; i < a->many_uint32_count; i++) COMPARE(many_uint32[i]);

  COMPARE(many_sint32_count)
  for (unsigned i = 0; i < a->many_sint32_count; i++) COMPARE(many_sint32[i]);

  COMPARE(many_int64_count)
  for (unsigned i = 0; i < a->many_int64_count; i++) COMPARE(many_int64[i]);

  COMPARE(many_uint64_count)
  for (unsigned i = 0; i < a->many_uint64_count; i++) COMPARE(many_uint64[i]);

  COMPARE(many_sint64_count)
  for (unsigned i = 0; i < a->many_sint64_count; i++) COMPARE(many_sint64[i]);

  COMPARE(many_fixed32_count)
  for (unsigned i = 0; i < a->many_fixed32_count; i++) COMPARE(many_fixed32[i]);

  COMPARE(many_fixed64_count)
  for (unsigned i = 0; i < a->many_fixed64_count; i++) COMPARE(many_fixed64[i]);

  COMPARE(many_str_count)
  for (unsigned i = 0; i < a->many_str_count; i++) COMPARE_STR(many_str[i]);

  COMPARE(many_bytes_count)
  for (unsigned i = 0; i < a->many_bytes_count; i++) COMPARE_BYTES(many_bytes[i]);

  COMPARE(many_bool_count)
  for (unsigned i = 0; i < a->many_bool_count; i++) COMPARE(many_bool[i]);

  COMPARE(many_float_count)
  for (unsigned i = 0; i < a->many_float_count; i++) COMPARE(many_float[i]);

  COMPARE(many_double_count)
  for (unsigned i = 0; i < a->many_double_count; i++) COMPARE(many_double[i]);

  COMPARE(many_sfixed32_count)
  for (unsigned i = 0; i < a->many_sfixed32_count; i++) COMPARE(many_sfixed32[i]);

  COMPARE(many_fixed64_count)
  for (unsigned i = 0; i < a->many_sfixed64_count; i++) COMPARE(many_sfixed64[i]);

  COMPARE(has_one_msg)
  if (a->has_one_msg && !compare_two_parentmsgs(&a->one_msg, &b->one_msg)) {
    return false;
  }

  COMPARE(many_msg_count)
  for (unsigned i = 0; i < a->many_msg_count; i++) {
    if (!compare_two_parentmsgs(&a->many_msg[i], &b->many_msg[i])) {
      return false;
    }
  }

  COMPARE(which_selector)
  switch (a->which_selector) {
    case PB_TAG_AllTypes_selected_int32: COMPARE(selected_int32) break;
    case PB_TAG_AllTypes_selected_uint32: COMPARE(selected_uint32) break;
    case PB_TAG_AllTypes_selected_sint32: COMPARE(selected_sint32) break;
    case PB_TAG_AllTypes_selected_int64: COMPARE(selected_int64) break;
    case PB_TAG_AllTypes_selected_uint64: COMPARE(selected_uint64) break;
    case PB_TAG_AllTypes_selected_sint64: COMPARE(selected_sint64) break;
    case PB_TAG_AllTypes_selected_fixed32: COMPARE(selected_fixed32) break;
    case PB_TAG_AllTypes_selected_fixed64: COMPARE(selected_fixed64) break;
    case PB_TAG_AllTypes_selected_bool: COMPARE(selected_bool) break;
    case PB_TAG_AllTypes_selected_str: COMPARE_STR(selected_str) break;
    case PB_TAG_AllTypes_selected_bytes: COMPARE_BYTES(selected_bytes) break;
    case PB_TAG_AllTypes_selected_float: COMPARE(selected_float) break;
    case PB_TAG_AllTypes_selected_double: COMPARE(selected_double) break;
    case PB_TAG_AllTypes_selected_sfixed32: COMPARE(selected_sfixed32) break;
    case PB_TAG_AllTypes_selected_sfixed64: COMPARE(selected_sfixed64) break;
    case PB_TAG_AllTypes_selected_msg: 
      if (!compare_two_parentmsgs(&a->selected_msg, &b->selected_msg)) {
        return false;
      }
      break;
  }

  return true;
}

int main(int argc, char *argv[]) {
  int opt;
  bool write_to_memory = false;
  bool compare = false;
  int testcase = -1;
  const char *filename = "test_EveryType.out";

  while ((opt = getopt(argc, argv, "mt:f:r")) != -1) {
    switch (opt) {
    case 'm':
      write_to_memory = true;
      break;
    case 'r':
      compare = true;
      break;
    case 't':
      testcase = atoi(optarg);
      break;
    case 'f':
      filename = optarg;
      break;
    default:
      fprintf(
        stderr, "usage: %s [-m] [-r] [-t testcase] [-f filename]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }


  struct AllTypes msg = build_message(testcase);

  if (compare) {
    /* Read the testcase from a file and compare it */
    FILE *infile = fopen(filename, "rb");
    if (!infile) {
      perror("fopen");
      return EXIT_FAILURE;
    }

    struct AllTypes inmsg = { 0 };
    if (!pb_decode_AllTypes(&inmsg, file_read, infile)) {
      fprintf(stderr, "Failed to parse message!\n");
      return EXIT_FAILURE;
    }

    if ((testcase > 0) && !compare_two_alltypes(&msg, &inmsg)) {
      return EXIT_FAILURE;
    }
  } else {
    /* Write the testcase to a file */
    FILE *outfile = fopen(filename, "wb");
    if (!outfile) {
      perror("fopen");
      return EXIT_FAILURE;
    }

    if (write_to_memory) {
      uint8_t buffer[PB_MAX_SIZE_AllTypes];
      unsigned length = pb_encode_AllTypes_to_buffer(buffer, &msg);
      fwrite(buffer, 1, length, outfile);
    } else {
      if (!pb_encode_AllTypes(&msg, file_write, outfile)) {
        return EXIT_FAILURE;
      }
    }
    fclose(outfile);
  }

  return EXIT_SUCCESS;
}
