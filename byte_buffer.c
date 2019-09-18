#include "byte_buffer.h"

#include <stdbool.h>

void bytebuf_init(bytebuf_t *bb,
    uint8_t *buf,
    size_t buflen,
    bytebuf_endianness_t endianness)
{
  *bb = (bytebuf_t){
    .start = buf,
    .size = buflen,
    .endianness = endianness,
  };
}

void bytebuf_write_1(bytebuf_t *bb, uint8_t byte)
{
  if (bb->pos < bb->size) {
    bb->start[bb->pos++] = byte;
  } else {
    bb->overflow++;
  }
}

uint8_t bytebuf_read_1(bytebuf_t *bb)
{
  if (bb->pos >= bb->size) {
    bb->underflow++;
    return 0;
  }
  return bb->start[bb->pos++];
}

static bool ensure_null_terminator(bytebuf_t *bb)
{
  if (!bb->pos)
    return false;

  if (bb->start[bb->pos - 1] == 0x0)
    return true;

  if (bb->pos < bb->size) {
    bb->start[bb->pos++] = 0x0;
  } else {
    bb->start[bb->pos - 1] = 0x0; // truncate
    bb->overflow++;
  }
  return true;
}

char *bytebuf_string(bytebuf_t *bb)
{
  return ensure_null_terminator(bb) ?
    (char*)bb->start : NULL;
}
