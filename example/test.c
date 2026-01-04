#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "example.pb.h"

int main(void) {

  struct MyMessage msg = {
    .some_integer = 5,
    .some_fixed = 5,
    .has_some_submsg = true,
    .some_submsg = {
      .type = MyEnum_AnotherValue,
      .some_text = { .str = "Hello, World", .len = 12 },
    },

    .many_integers = {1, 2, 3, 4},
    .many_integers_count = 4,

    .many_fixed = {1, 2, 3, 4},
    .many_fixed_count = 4,

    .many_submsg_count = 2,
    .many_submsg = {
      {
        .type = MyEnum_AnotherValue,
        .some_text = { .str = "Hello, two", .len = 10 },
      },
      {
        .type = MyEnum_AnotherValue,
        .some_text = { .str = "Hello, three", .len = 12 },
      },
    },
  };

  uint8_t buffer[PB_MAX_SIZE_MyMessage];
  unsigned len = pb_encode_MyMessage_to_buffer(buffer, &msg);

  FILE *f = fopen("test.out", "wb");
  fwrite(buffer, 1, len, f);
  fclose(f);
  printf("Wrote %d bytes to test.out\n", len);


  struct pb_buffer buf = { .ptr = buffer, .length = sizeof(buffer) };
  bool result = pb_encode_MyMessage(&msg, pb_buffer_write, &buf);
  if (!result) {
    printf("Failed to encode!\n");
  } else {
    FILE *f = fopen("test2.out", "wb");
    fwrite(buffer, 1, buf.bytes_used, f);
    fclose(f);
    printf("Wrote %d bytes to test.out\n", buf.bytes_used);
  }

  struct MyMessage out = { 0 };
  struct pb_buffer rbuf = { .ptr = buffer, .length = buf.bytes_used };
  bool result2 = pb_decode_MyMessage(&out, pb_buffer_read, &rbuf);
  printf("%d\n", (int)result2);

  return 0;
}
