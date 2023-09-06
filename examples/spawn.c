#include <stdint.h>
#include <string.h>

#include "ckb_syscalls.h"

int main() {
  const char *argv[] = {"-e", "local m = arg[2] .. arg[3]; ckb.set_content(m)",
                        "hello", "world"};
  uint8_t content[80] = {};
  uint64_t content_length = 80;
  int8_t exit_code = -1;

  spawn_args_t spgs = {
      .memory_limit = 8,
      .exit_code = &exit_code,
      .content = content,
      .content_length = &content_length,
  };

  int success = ckb_spawn(1, 3, 0, 4, argv, &spgs);
  if (success != 0) {
    return 1;
  }
  if (exit_code != 0) {
    return 1;
  }
  if (strlen((char *)content) != 10) {
    return 1;
  }
  if (strcmp((char *)content, "helloworld") != 0) {
    return 1;
  }
  return 0;
}
