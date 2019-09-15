#pragma once

#define DEBUG(lvl, fmt, ...)\
  fprintf(stderr, fmt "\n", ##__VA_ARGS__);

#define RASSERT(expr, ...)\
  do { \
    if (!(expr)) { \
      DEBUG(1, "Assertion failed: " #expr ": " __VA_ARGS__); \
      abort(); \
    } \
  } while(0)

// TODO: disable in release build.
#define DASSERT(expr, ...) UTF_RASSERT(expr, __VA_ARGS__)

#include <stdio.h>
#include <stdlib.h>
