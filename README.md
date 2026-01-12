## ViaPB

ViaPB is a very minimal C code generator for protocol buffers.  It
is inspired from nanopb but with a focus on the performance of generated code,
and sacrifices most of the flexibility that nanopb offers.  Such compromises as:
 - No callbacks or pointer values
 . All BYTES and STRING types are static bounded arrays
 - All REPEATED types are static bounded arrays
 - No ability to customize C naming convention or types
 - Only support proto3: No default values, label optional is default, no required fields
 - Requires C11 (for anonymous unions), only tested with GCC
 - Relies heavily on automatic inlining, at a significant text size increase
 - Doesn't support multiple proto compilation units depending on each other: Entire descriptor set compiled to one source/header pair
 - In general, no features that I won't directly use

This results in fixed-size structs for messages, and thus all messages have a
known max size both serialized and in memory

## Usage
All repeated fields need a max count specified, and string/bytes fields need a max size specified.  This is done in an options file.  Each line of the
options file starts with a field path (delimited with `.`) and then can specify max_count and max_size:
```
.SubMessage.some_text       max_size:16
.MyMessage.many_submsg      max_count:4
```

Additionally, the path to a message type can have an option `submsg_store`, which will inline the serialization
of any submessages that are smaller than the given size.  This can increase performance, at the cost of increasing
the stack frame usage by the provided amount.

Use protoc to generate a binary FileDescriptorSet:
```             
protoc --include_imports -I proto -o out mymessages.proto
```


Pass the FileDescriptorSet to stdin:
```
python generate.py --file out --options viapb.options
```


For each proto file in the set, it will create a protofile.pb.h and
protofile.pb.c, which in turn include viapb.h. Each C object exports four
functions for each message:
 - `size_t pb_sizeof_MessageName(const struct MessageName *)` - returns the encoded
   size of the message, more efficiently than encoding the message.
 - `bool pb_encode_MessageName(const struct MessageName *, pb_write_fn callback, void *ptr)` -
  encodes the struct, calling `pb_write_fn` with the contents of the serialized message.  The write callback will be called repeatedly with
  small chunks, and ptr will be passed to the callback. False is returned if the callback returns false, or if an issue prevents full serialization of
  the message, such as an invalid size/length or enum value exceeding range.
 - `bool pb_encode_MessageName_to_buffer(uint8_t data[N], const struct MessageName *)` - Encodes the message to the passed data. N is replaced with
   the max serialized size of the message. False is returned is an issue prevented full serialization of the message, an invalid size/length or enum
   value exceeding range
 - `bool pb_decode_MessageName(struct MessageName *, pb_read_fn callback, void *ptr)` - Decode a message read using pb_read_fn into the provided
   message struct.  Returns false if decoding failed at any stage other than reading the next field of a message.

As an example, a `pb_write_fn` implementation for static arrays is provided, `pb_buffer_write`:
```
uint8_t buffer[PB_MAX_SIZE_MyMessage];
struct pb_buffer writer = { .ptr = buffer, .length = sizeof(buffer) };
struct MyMessage msg = { 0 };
bool success = pb_encode_MyMessage(&msg, pb_buffer_write, &writer);
      // writer.bytes_used now contains the number of bytes used
```

For serializing an entire message, using the _to_buffer encoders will be significantly faster due to lack of internal copies and use of callbacks.

Likewise, a `pb_buffer_read` is provided to decode from a buffer:
```
uint8_t buffer[PB_MAX_SIZE_MyMessage] = { ... };
struct pb_buffer reader = { .ptr = buffer, .length = sizeof(buffer) };
struct MyMessage msg = { 0 };
bool success = pb_decode_MyMessage(&msg, pb_buffer_read, &reader);
      // reader.bytes_used now contains the number of bytes read from the buffer
```

## Licensing
This project is licensed under the zlib license, found in LICENSE.zlib.  The generated code can be licensed in any way, though it includes the two
headers found in this repo, which are zlib licensed.

## Stuff
 No automation, but:
 - black for formatting
 - mypy for type checking
 - example/ has a simple example on usage with a C program. The generated output
   is commited to provide an example without having to clone/build
 - tests/ has (the start of) an integration test to validate encode/decoder correctness. With all the python requirements installed, run `make
   validate` in the tests directory
