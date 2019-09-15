#pragma once

typedef enum {
  BF_ERROR_SUCCESS = 0,
  BF_ERROR_FAILURE = 1,
  BF_ERROR_OUT_OF_MEMORY = 2,
  BF_ERROR_SYNTAX = 3,
  BF_ERROR_END_OF_FILE = 4,
} bf_error_t;
