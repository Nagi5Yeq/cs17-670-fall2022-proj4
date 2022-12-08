#pragma once

#include <stdint.h>

typedef uint8_t byte;

typedef enum {
  I32 = 0x7f,
  F64 = 0x7c,
  EXTERNREF = 0x6f,
  FUNCREF = 0x70
} wasm_type_t;

typedef enum { FUNC, GLOBAL, TABLE, MEMORY } wasm_import_kind_t;

typedef struct {
  union {
    uint32_t i32;
    double f64;
    void* ref;
    uint64_t i64;
  } val;
} wasm_value_t;

typedef struct {
  uint64_t tag;
  wasm_value_t val;
} wasm_typed_value_t;

typedef struct {
  int32_t length;  // < 0 indicates a trap or error
  wasm_value_t* vals;
} wasm_values;

wasm_value_t parse_wasm_value(char* string);
void print_wasm_value(wasm_typed_value_t val);
void trace_wasm_value(wasm_typed_value_t val);

wasm_value_t wasm_i32_value(int32_t val);
wasm_value_t wasm_f64_value(double val);
wasm_value_t wasm_ref_value(void* val);
