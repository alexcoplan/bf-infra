#include "bf_interpreter.h"
#include "debug.h"

int main(void)
{
  uint8_t tape[30 * 1000];
  const bf_interpreter_params_t params = {
    .mem = tape,
    .mem_size = sizeof(tape),
  };

  const char *hello_world = "+[-[<<[+[--->]-[<<<]]]>>>-]>-.---.>..>.<<<<-.<+.>>>>>.>.<<.<-.";

  bf_interpreter_t *bfi;
  RASSERT(bf_interpreter_create(&bfi, hello_world, &params) == BF_ERROR_SUCCESS);
  RASSERT(bf_interpreter_run(bfi) == BF_ERROR_SUCCESS);
}
