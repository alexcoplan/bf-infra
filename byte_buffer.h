#pragma once

#include <stdint.h>
#include <stddef.h>

#define BYTEBUF_DEFINE_LOCAL(name, size, endian)\
  uint8_t name##_storage[size];\
  bytebuf_t name;\
  bytebuf_init(&name, name##_storage, (size), (endian));

typedef enum {
  BYTEBUF_LITTLE_ENDIAN,
  BYTEBUF_BIG_ENDIAN,
  BYTEBUF_NETWORK_ENDIAN = BYTEBUF_BIG_ENDIAN,
#ifdef __LITTLE_ENDIAN__
#ifdef __BIG_ENDIAN__
#error both little and big?
#endif
  BYTEBUF_HOST_ENDIAN = BYTEBUF_LITTLE_ENDIAN,
#else
#ifndef __BIG_ENDIAN__
#error neither little nor big?
#endif
  BYTEBUF_HOST_ENDIAN = BYTEBUF_BIG_ENDIAN,
#endif
} bytebuf_endianness_t;

typedef struct bytebuf {
  uint8_t *start;
  size_t pos;
  size_t size;
  size_t overflow;
  size_t underflow;
  bytebuf_endianness_t endianness;
} bytebuf_t;

void bytebuf_init(bytebuf_t *bb,
    uint8_t *buf,
    size_t buflen,
    bytebuf_endianness_t endian);

void bytebuf_write_1(bytebuf_t *bb, uint8_t byte);

uint8_t bytebuf_read_1(bytebuf_t *bb);

char *bytebuf_string(bytebuf_t *bb);
