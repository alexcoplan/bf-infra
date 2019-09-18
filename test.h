#pragma once

#include "macros.h"

#include <stddef.h>
#include <stdbool.h>
#include <string.h>

typedef bool (*test_fn_t)(void);

typedef struct {
  const char *names;
  const test_fn_t *test_fns;
  size_t n_fns;
} test_manifest_t;

#define TEST_PASS return true

#define TEST_DEFINE(name) static bool name(void)

#define RUN_TESTS(...)\
  static const test_fn_t test_fns_[] = { __VA_ARGS__ }; \
  const test_manifest_t test_manifest_ = { \
    .names = #__VA_ARGS__, \
    .test_fns = test_fns_, \
    .n_fns = ARRAY_LENGTH(test_fns_), \
  }; \
  int main(void) { return test_main(); }

#define TEST_FAIL(fmt, ...)\
  do { \
    test_fail("Assertion failed [%s:%d] " fmt, \
        __FILE__, __LINE__, __VA_ARGS__); \
    return false; \
  } while(0)

#define ASSERT_EQ(e_x, e_y) \
  do { \
    const int x = (e_x); \
    const int y = (e_y); \
    if (x != y) { \
      TEST_FAIL(#e_x " != " #e_y ": %d vs %d", \
          x, y); \
    } \
  } while (0)

#define ASSERT_STR_EQ(e_x, e_y) \
  do { \
    const char *x = (e_x); \
    const char *y = (e_y); \
    if (strcmp(x, y)) { \
      TEST_FAIL(#e_x " != " #e_y ": %s vs %s", \
          x, y); \
    } \
  } while (0)

void test_fail(const char *fmt, ...);

int test_main(void);
