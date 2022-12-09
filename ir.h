#pragma once

#include <stdint.h>

typedef uint8_t byte;

enum wasm_type_t { I32 = 0x7f, F64 = 0x7c, EXTERNREF = 0x6f, FUNCREF = 0x70 };

struct wasm_value_t {
  union {
    uint32_t i32;
    double f64;
    void* ref;
    uint64_t i64;
  };
};

struct wasm_typed_value_t {
  uint64_t tag;
  wasm_value_t val;
};

wasm_value_t parse_wasm_value(char* string);
void print_wasm_value(wasm_typed_value_t val);
void trace_wasm_value(wasm_typed_value_t val);

wasm_value_t wasm_i32_value(int32_t val);
wasm_value_t wasm_f64_value(double val);
wasm_value_t wasm_ref_value(void* val);
