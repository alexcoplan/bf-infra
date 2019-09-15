#pragma once

#include <stddef.h>
#include <stdint.h>
#include "error.h"

typedef struct bf_interpreter bf_interpreter_t;

typedef void (*bf_putc_cb_t)(uint8_t, void *arg);
typedef uint8_t (*bf_getc_cb_t)(void *arg);

typedef struct bf_interpreter_params {
  uint8_t *mem;
  size_t mem_size;

  bf_putc_cb_t putc_callback;
  bf_getc_cb_t getc_callback;
  void *user_data;
} bf_interpreter_params_t;

bf_error_t bf_interpreter_create(bf_interpreter_t **bfi,
    const char *code,
    const bf_interpreter_params_t *params);

bf_error_t bf_interpreter_step(bf_interpreter_t *bfi);
bf_error_t bf_interpreter_run(bf_interpreter_t *bfi);
