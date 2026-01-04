#ifndef VIAPB_H
#define VIAPB_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

typedef bool (*pb_write_fn)(const uint8_t *data, unsigned len, void *user);
typedef bool (*pb_read_fn)(uint8_t *data, unsigned len, void *user);

struct pb_buffer {
  uint8_t *ptr;
  unsigned length;
  unsigned bytes_used;
};

static bool pb_buffer_write(const uint8_t *data, unsigned len, void *user) {
  struct pb_buffer *w = (struct pb_buffer *)user;
  if (len > w->length) {
    return false;
  }
  memcpy(w->ptr, data, len);
  w->ptr += len;
  w->length -= len;
  w->bytes_used += len;
  return true;
}

static bool pb_buffer_read(uint8_t *data, unsigned len, void *user) {
  struct pb_buffer *w = (struct pb_buffer *)user;
  if (len > w->length) {
    return false;
  }
  memcpy(data, w->ptr, len);
  w->ptr += len;
  w->length -= len;
  w->bytes_used += len;
  return true;
}

#pragma GCC diagnostic pop

#endif
