#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "common.h"
#include "ir.h"
#include "weewasm.h"

extern uint32_t jit_enable;
extern uint32_t jit_check;

// Initialize the runtime functions.
void init_runtime();
void init_env_runtime();

// Disassembles? and runs a wasm module.
wasm_typed_value_t run(const byte* start,
                       const byte* end,
                       uint32_t num_args,
                       wasm_value_t* args);

// Main function.
// Parses arguments and either runs the tests or runs a file with arguments.
//  -trace: enable tracing to stderr
//  -test: run internal tests
int main(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    char* arg = argv[i];
    if (strcmp(arg, "-trace") == 0) {
      g_trace = 1;
      continue;
    }
    if (strcmp(arg, "-disassemble") == 0) {
      g_disassemble = 1;
      continue;
    }
    if (strcmp(arg, "-no-jit") == 0) {
      jit_enable = 0;
      continue;
    }
    if (strcmp(arg, "-no-jit-check") == 0) {
      jit_check = 0;
      continue;
    }

    byte* start = NULL;
    byte* end = NULL;
    ssize_t r = load_file(arg, &start, &end);
    if (r >= 0) {
      TRACE("loaded %s: %ld bytes\n", arg, r);
      int num_args = argc - i - 1;
      wasm_value_t* args = NULL;
      if (num_args > 0) {
        args = (wasm_value_t*)malloc(sizeof(wasm_value_t) * num_args);
        for (int j = i + 1; j < argc; j++) {
          int a = j - i - 1;
          args[a] = parse_wasm_value(argv[j]);
          TRACE("args[%d] = ", a);
          //     trace_wasm_value(args[a]);
          TRACE("\n");
        }
      }
      init_runtime();
      init_env_runtime();
      wasm_typed_value_t result = run(start, end, num_args, args);
      unload_file(&start, &end);
      print_wasm_value(result);
      printf("\n");
      exit(0);
      return 0;
    } else {
      ERR("failed to load: %s\n", arg);
      return 1;
    }
  }
  return 0;
}
