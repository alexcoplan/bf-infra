#include "bf_interpreter.h"
#include "test.h"
#include "byte_buffer.h"

static void bb_putc_cb(uint8_t byte, void *arg)
{
  bytebuf_t *bb = arg;
  bytebuf_write_1(bb, byte);
}

TEST_DEFINE(test_hello)
{
  BYTEBUF_DEFINE_LOCAL(bb, 32, BYTEBUF_HOST_ENDIAN);
  uint8_t tape[256] = { 0 };

  const bf_interpreter_params_t params = {
    .mem = tape,
    .mem_size = sizeof(tape),
    .putc_callback = bb_putc_cb,
    .user_data = &bb,
  };

  const char *hello_world = "+[-[<<[+[--->]-[<<<]]]>>>-]>-.---.>..>.<<<<-.<+.>>>>>.>.<<.<-.";

  bf_interpreter_t *bfi;
  ASSERT_EQ(bf_interpreter_create(&bfi,
        hello_world, &params), BF_ERROR_SUCCESS);
  ASSERT_EQ(bf_interpreter_run(bfi), BF_ERROR_SUCCESS);
  ASSERT_STR_EQ(bytebuf_string(&bb), "hello world");

  TEST_PASS;
}

RUN_TESTS(
  test_hello
)
