#pragma once

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "common.h"
#include "illegal.h"
#include "ir.h"
#include "weewasm.h"

#define WASM_TYPE_FUNCTION 0x60
#define WASM_TYPE_I32 0x7F
#define WASM_TYPE_I64 0x7E
#define WASM_TYPE_F64 0x7C
#define WASM_TYPE_FUNCREF 0x70
#define WASM_TYPE_EXTERNREF 0x6F

struct wasm_func_decl_t {
  std::vector<uint8_t> args_;
  std::vector<uint8_t> ret_;
};

struct wasm_table_decl_t {
  uint8_t type_;
  byte has_max_;
  uint32_t init_;
  uint32_t max_;
};

struct wasm_memory_decl_t {
  byte has_max_;
  uint32_t init_;
  uint32_t max_;
};

struct wasm_global_decl_t {
  uint8_t type_;
  byte is_mut_;
};

struct wasm_import_decl_t {
  enum class type { FUNCTION = 0x0, TABLE = 0x1, MEMORY = 0x2, GLOBAL = 0x3 };

  type type_;
  std::string module_;
  std::string name_;
  union {
    uint32_t funcindex_;
    wasm_table_decl_t table_;
    wasm_memory_decl_t memory_;
    wasm_global_decl_t global_;
  };
};

struct wasm_global_t {
  wasm_global_decl_t type_;
  union {
    uint32_t i32;
    double f64;
    void* ref;
  } val_;
};

struct wasm_export_t {
  std::string name_;
  uint32_t type_;
  uint32_t idx_;
};

struct wasm_elem_decl_t {
  int32_t offset_;
  std::vector<uint32_t> funcidx_;
};

struct wasm_local_decl_t {
  uint32_t count_;
  uint32_t type_;
};

struct wasm_code_decl_t {
  uint32_t size_;
  std::vector<wasm_local_decl_t> locals_;
  std::vector<byte> code_;
};

struct wasm_data_decl_t {
  uint32_t offset_;
  std::vector<byte> data_;
};

struct wasm_instance_t;

struct wasm_func_t {
  wasm_func_decl_t* sig_;
  uint32_t typeidx_;
  std::vector<wasm_local_decl_t> locals_;
  std::vector<byte> code_;

  // run imported function in interperter
  void (*run_)(wasm_instance_t* ins);
  // run imported function in JIT
  void* run_jit_;
};

struct wasm_module_t {
  wasm_memory_decl_t mem_limits_;
  wasm_table_decl_t table_;
  uint32_t num_import_funcs_;
  uint32_t main_index_;
  bool have_start_;
  uint32_t start_index_;
  std::vector<wasm_func_decl_t> sigs_;
  std::vector<wasm_func_t> funcs_;
  std::vector<wasm_global_t> globals_;
  std::vector<wasm_data_decl_t> datas_;
  std::vector<wasm_elem_decl_t> elems_;

  wasm_instance_t* create_instance();
};

const uint32_t wasm_page_size = 65536;

struct wasm_stack_frame_t {
  buffer_t pc_;
  wasm_func_t* f_;
  std::vector<wasm_value_t> locals_;
};

struct wasm_jit_table_entry_t {
  int64_t typeidx;
  uint8_t* addr;
};

struct wasm_instance_t {
  wasm_module_t* module_;
  std::vector<byte> memory_;
  std::vector<wasm_func_t*> table_;
  std::vector<wasm_jit_table_entry_t> jit_table_;
  std::vector<wasm_value_t> globals_;
  std::vector<wasm_value_t> value_stack_;
  std::vector<wasm_stack_frame_t> frames_;
  wasm_stack_frame_t* cur_frame_;
  uint32_t trapped;

  void run_function(uint32_t funcidx);
  wasm_value_t run(std::vector<wasm_value_t> args);
  wasm_value_t run_jit(std::vector<wasm_value_t> args);
  void run_fn(uint8_t* f,
              size_t num_args,
              wasm_value_t* args,
              uint8_t* arg_types,
              wasm_value_t* r,
              int r_type);
  void trap(uint32_t reason);

  uint32_t read_u32leb();
  int32_t read_i32leb();
  double read_double();
  byte read_byte();

  void push_value(wasm_value_t v);
  wasm_value_t pop_value();
  void branch(uint32_t to_off);

  void push_frame(wasm_func_t* f);
  void pop_frame();

  void ensure_memory(uint32_t offset);
  void ensure_table(uint32_t offset);
};

#define WASM_TRAP_REASON_NOTRAP 0
#define WASM_TRAP_REASON_FUNCEND 1
#define WASM_TRAP_REASON_SIGFPE 2
#define WASM_TRAP_REASON_OOB 3
#define WASM_TRAP_REASON_TYPEERR 4
#define WASM_TRAP_REASON_UNREACHABLE 5
#define WASM_TRAP_REASON_NULLFUNCREF 6
#define WASM_TRAP_REASON_NULLEXTERNREF 7
#define WASM_TRAP_REASON_BADOBJECT 8

#define WASM_INST_TYPE_NOARG 0
#define WASM_INST_TYPE_BLOCK 1
#define WASM_INST_TYPE_I32 2
#define WASM_INST_TYPE_F64 3
#define WASM_INST_TYPE_MEMARG 4
#define WASM_INST_TYPE_BRTABLE 5
#define WASM_INST_TYPE_I32_I32 6
#define WASM_INST_TYPE_S32 7

struct wasm_jit_t;

struct wasm_inst_desc_t {
  byte opcode_;
  byte type_;
  const char* name_;
  void (*run)(wasm_instance_t* ins, byte opcode);
  void (wasm_jit_t::*emit)();
};

extern const struct wasm_inst_desc_t g_inst_desc[];

struct wasm_rt_func_t {
  void (*run_)(wasm_instance_t* ins);
  void* run_jit_;
};
extern std::unordered_map<std::string, wasm_rt_func_t> g_runtime_funcs;

#define WASM_EXTERN_NULL 0
#define WASM_EXTERN_OBJECT 1
#define WASM_EXTERN_STRING 2
#define WASM_EXTERN_BOXI32 3
#define WASM_EXTERN_BOXF64 4

struct wasm_externref_t {
  uint32_t type_;
  // make it not move constructible
  wasm_externref_t(wasm_externref_t&&) = delete;
  wasm_externref_t(uint32_t type) : type_(type){};
};

struct wasm_e_string_t : public wasm_externref_t {
  std::string str_;
  wasm_e_string_t(const char* str)
      : wasm_externref_t(WASM_EXTERN_STRING), str_(str) {}
};

typedef std::pair<uint32_t, uint64_t> wasm_key_t;

struct wasm_e_object_t : public wasm_externref_t {
  std::map<wasm_key_t, wasm_externref_t*> map_;
  wasm_e_object_t() : wasm_externref_t(WASM_EXTERN_OBJECT) {}
};

struct wasm_e_box_t : public wasm_externref_t {
  union {
    int32_t i32_;
    double f64_;
  };
  wasm_e_box_t(uint32_t type) : wasm_externref_t(type) {}
};

extern wasm_instance_t* active_instance;
extern uint32_t jit_enable;
extern uint32_t jit_check;
