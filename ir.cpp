#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "ir.h"

wasm_value_t parse_wasm_value(char* str) {
  int len = strlen(str);
  if (len > 0) {
    if (str[len - 1] == 'd' || str[len - 1] == 'D') {
      // treat the input as a double
      char* end = NULL;
      double result = strtod(str, &end);
      if (end == (str + len - 1))
        return wasm_f64_value(result);
    } else {
      // treat the input as an integer
      char* end = NULL;
      int base = 10;
      if (len >= 2 && (str[1] == 'x' || str[1] == 'X'))
        base = 16;
      long result = strtol(str, &end, base);
      if (end == (str + len))
        return wasm_i32_value(result);
    }
  }
  wasm_value_t orig_string;
  orig_string.ref = str;
  return orig_string;
}

void print_wasm_value(wasm_typed_value_t val) {
  switch (val.tag) {
    case I32:
      printf("%d", val.val.i32);
      break;
    case F64:
      printf("%lf", val.val.f64);
      break;
    case EXTERNREF:
      if (val.val.ref == NULL)
        printf("null");
      else
        printf("%p", val.val.ref);
      break;
  }
}

void trace_wasm_value(wasm_typed_value_t val) {
  switch (val.tag) {
    case I32:
      TRACE("%d", val.val.i32);
      break;
    case F64:
      TRACE("%lf", val.val.f64);
      break;
    case EXTERNREF:
      if (val.val.ref == NULL)
        printf("null");
      else
        TRACE("%p", val.val.ref);
      break;
  }
}

wasm_value_t wasm_i32_value(int32_t val) {
  wasm_value_t r;
  r.i32 = val;
  return r;
}

wasm_value_t wasm_f64_value(double val) {
  wasm_value_t r;
  r.f64 = val;
  return r;
}

wasm_value_t wasm_ref_value(void* val) {
  wasm_value_t r;
  r.ref = val;
  return r;
}
