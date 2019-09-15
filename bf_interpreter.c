#include "bf_interpreter.h"
#include "error.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ARRAY_LENGTH(x) ((sizeof(x)) / (sizeof((x)[0])))

struct bf_interpreter {
  uint8_t *tape;
  size_t tape_size;

  uint8_t *code;
  size_t code_size;

  size_t code_pos;
  size_t tape_pos;

  bf_putc_cb_t putc_cb;
  bf_getc_cb_t getc_cb;
  void *udata;
};

typedef enum {
  BF_INST_INC        = '+',
  BF_INST_DEC        = '-',
  BF_INST_RIGHT      = '>',
  BF_INST_LEFT       = '<',
  BF_INST_LOOP_START = '[',
  BF_INST_LOOP_END   = ']',
  BF_INST_PUTC       = '.',
  BF_INST_GETC       = ',',

  BF_INST_MAX,
} bf_inst_type_t;

// In-memory instruction representation:
//  - 8-bit type.
//  - If type is LOOP_START or LOOP_END, size_t offset.
static size_t instruction_size(bf_inst_type_t type)
{
  switch (type) {
    case BF_INST_INC:
    case BF_INST_DEC:
    case BF_INST_RIGHT:
    case BF_INST_LEFT:
    case BF_INST_PUTC:
    case BF_INST_GETC:
      return 1;
    case BF_INST_LOOP_START:
    case BF_INST_LOOP_END:
      return 1 + sizeof(size_t);
  }

  return 0;
}

static size_t calculate_code_size(const char *code)
{
  size_t required = 0;
  for (size_t i = 0; code[i]; i++) {
    required += instruction_size(code[i]);
  }

  return required;
}

static bf_error_t build_instruction_memory(bf_interpreter_t *bfi, const char *code)
{
  size_t code_pos = 0, jump_count = 0;
  size_t jump_stack[1024];

  for (size_t i = 0; code[i]; i++) {
    const bf_inst_type_t inst = code[i];
    const size_t inst_size = instruction_size(inst);
    if (!inst_size)
      continue;

    RASSERT(code_pos + inst_size <= bfi->code_size);
    bfi->code[code_pos] = inst;

    if (code[i] == BF_INST_LOOP_START) {
      RASSERT(jump_count < ARRAY_LENGTH(jump_stack) && "exceeded max loop depth");
      jump_stack[jump_count++] = code_pos;
    } else if (code[i] == BF_INST_LOOP_END) {
      if (!jump_count) {
        DEBUG(1, "unmatched ']'");
        return BF_ERROR_SYNTAX;
      }

      const size_t loop_start = jump_stack[jump_count-1];
      memcpy(bfi->code + loop_start + 1, &code_pos, sizeof(size_t));
      memcpy(bfi->code + code_pos + 1, &loop_start, sizeof(size_t));
      jump_count--;
    }

    code_pos += inst_size;
  }

  if (jump_count) {
    DEBUG(1, "unmatched '['");
    return BF_ERROR_SYNTAX;
  }
  RASSERT(code_pos == bfi->code_size);

  return BF_ERROR_SUCCESS;
}

static void default_putc_cb(uint8_t ch, void *arg)
{
  RASSERT(putchar(ch) == ch);
}

static uint8_t default_getc_cb(void *arg)
{
  return getchar();
}

bf_error_t bf_interpreter_create(bf_interpreter_t **out,
    const char *code,
    const bf_interpreter_params_t *params)
{
  const size_t code_size = calculate_code_size(code);
  const size_t to_alloc = sizeof(struct bf_interpreter) + code_size;

  bf_interpreter_t *bfi = malloc(to_alloc);
  if (!bfi)
    return BF_ERROR_OUT_OF_MEMORY;

  *bfi = (bf_interpreter_t) {
    .code = (uint8_t *)(bfi + 1),
    .code_size = code_size,
    .tape = params->mem,
    .tape_size = params->mem_size,
    .putc_cb = params->putc_callback ? params->putc_callback : default_putc_cb,
    .getc_cb = params->getc_callback ? params->getc_callback : default_getc_cb,
    .udata = params->user_data,
  };

  bf_error_t err = build_instruction_memory(bfi, code);
  if (err) {
    free(bfi);
  } else {
    *out = bfi;
  }

  return err;
}

bf_error_t bf_interpreter_step(bf_interpreter_t *bfi)
{
  if (bfi->code_pos >= bfi->code_size) {
    RASSERT(bfi->code_pos == bfi->code_size);
    return BF_ERROR_END_OF_FILE;
  }

  const bf_inst_type_t inst = bfi->code[bfi->code_pos];
  const size_t inst_size = instruction_size(inst);

  bool loop_start = 0;
  switch (inst) {
    case BF_INST_INC:
      bfi->tape[bfi->tape_pos]++;
      break;
    case BF_INST_DEC:
      bfi->tape[bfi->tape_pos]--;
      break;
    case BF_INST_RIGHT:
      RASSERT(bfi->tape_pos+1 < bfi->tape_size);
      bfi->tape_pos++;
      break;
    case BF_INST_LEFT:
      RASSERT(bfi->tape_pos > 0);
      bfi->tape_pos--;
      break;
    case BF_INST_PUTC:
      bfi->putc_cb(bfi->tape[bfi->tape_pos], bfi->udata);
      break;
    case BF_INST_GETC:
      bfi->tape[bfi->tape_pos] = bfi->getc_cb(bfi->udata);
      break;
    case BF_INST_LOOP_START:
      loop_start = 1;
    case BF_INST_LOOP_END:
      if (loop_start == !bfi->tape[bfi->tape_pos]) {
        memcpy(&bfi->code_pos, bfi->code + bfi->code_pos + 1, sizeof(bfi->code_pos));
        return BF_ERROR_SUCCESS;
      }
      break;
    default:
      RASSERT(0, "bad instruction 0x%x = '%c'", inst, inst);
  }

  bfi->code_pos += inst_size;
  return BF_ERROR_SUCCESS;
}

bf_error_t bf_interpreter_run(bf_interpreter_t *bfi)
{
  bf_error_t err;
  while (!(err = bf_interpreter_step(bfi)));
  return (err == BF_ERROR_END_OF_FILE) ? BF_ERROR_SUCCESS : err;
}
