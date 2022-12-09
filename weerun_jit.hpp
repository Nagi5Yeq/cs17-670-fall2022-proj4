#pragma once

#include <array>
#include <map>
#include <utility>
#include <vector>

#include "weerun.hpp"

#include <xbyak/xbyak.h>
#include <xbyak/xbyak_bin2hex.h>

namespace reg {
enum reg {
  rax = 0,
  rdx = 1,
  rcx = 2,
  rdi = 3,
  rsi = 4,
  r8 = 5,
  r9 = 6,
  r10 = 7,
  r11 = 8,
  // callee-saved:
  rbx = 9,
  r12 = 10,
  r13 = 11,
  r14 = 12,
  r15 = 13,

  xmm0 = 14,
  xmm1 = 15,
  xmm2 = 16,
  xmm3 = 17,
  xmm4 = 18,
  xmm5 = 19,
  xmm6 = 20,
  xmm7 = 21,
  xmm8 = 22,
  xmm9 = 23,
  xmm10 = 24,
  xmm11 = 25,
  xmm12 = 26,
  xmm13 = 27,
  xmm14 = 28,
  xmm15 = 29,

  num_r = 30
};

const int32_t in_reg = 0;
const int32_t on_stack = 1;
}  // namespace reg

struct wasm_jit_loc_t {
  int32_t type;   // 0:in register, 1:on stack
  int32_t index;  // register index or [rbp-8*index]

  bool operator==(const wasm_jit_loc_t& rhs) {
    return type == rhs.type && index == rhs.index;
  }
};

enum class wasm_jit_type_t : int32_t {
  INVALID,
  CALLER_DATA,
  INTERMEDIATE,
  LOCAL_VAL,  // index = local variable index
  STACK_VAL,  // index = stack index
  GLOBAL_BASE,
  MEMORY_BASE,
  MEMORY_SIZE,
};

struct wasm_jit_val_t {
  wasm_jit_type_t type;
  int32_t index;

  bool operator==(const wasm_jit_val_t& rhs) {
    return type == rhs.type && index == rhs.index;
  }
  bool operator!=(const wasm_jit_val_t& rhs) {
    return type != rhs.type || index != rhs.index;
  }
};

struct wasm_jit_ctx_t {
  std::array<wasm_jit_val_t, 30> reg_;  // reg content
  std::array<uint32_t, 30> reg_age_;    // reg content
  uint32_t age_;
  std::vector<uint8_t> stack_valid_;  // stack valid?

  std::vector<wasm_jit_loc_t> stack_loc_;  // location of wasm stack values
  std::vector<wasm_jit_loc_t> local_loc_;  // location of wasm local values

  void init(wasm_jit_t* jit, wasm_func_t* f);
};

struct wasm_jit_t : public Xbyak::CodeGenerator {
  wasm_module_t* mod_;
  wasm_instance_t* ins_;
  int max_prologue_size_;
  bool enable_check_;
  std::vector<uint8_t*> f_addr_;
  uint8_t* run_fn_;  // call helper function

  // current function info
  wasm_func_t* f_;
  uint32_t funcidx_;
  uint32_t num_locals_;
  std::vector<uint8_t> local_types_;
  uint32_t rsp_sub_;
  std::array<uint8_t, 5> used_callee_;

  // decoder info
  uint32_t opcode_;
  buffer_t buf_;
  uint32_t next_bi_;
  uint32_t stop_;

  // register allocation context
  wasm_jit_ctx_t ctx_;

  // branch info
  struct block_t {
    uint32_t loop;
    uint32_t index;
    wasm_jit_ctx_t saved_ctx;  // for loop, we need to save context early
    std::vector<wasm_jit_ctx_t> br_ctxs;  // brs' cpntext
  };
  std::vector<block_t> block_stack_;

  wasm_jit_t(uint64_t code_size, bool check);

  void compile_instance(wasm_instance_t* ins);
  void emit_runtime_functions();
  void compile_function(uint32_t index);
  void run_fn(uint32_t index, wasm_value_t* args, wasm_value_t* r);

  Xbyak::Reg64 to_xbyak_r64(int32_t index);
  Xbyak::Reg32 to_xbyak_r32(int32_t index);
  Xbyak::Reg16 to_xbyak_r16(int32_t index);
  Xbyak::Reg8 to_xbyak_r8(int32_t index);
  Xbyak::Xmm to_xbyak_xmm(int32_t index);

  int32_t alloc_r(wasm_jit_val_t usage, uint8_t type);
  void acquire_r(int32_t index, wasm_jit_val_t usage);
  void use_r(int32_t index, wasm_jit_val_t usage);
  void release_r(int32_t index);  // mark a register as invalid

  int32_t alloc_stk();
  void release_stk(int32_t index);
  void acquire_stk(int32_t index);

  inline wasm_jit_loc_t peek_wasm_stk(uint32_t depth);
  inline wasm_jit_loc_t pop_wasm_stk();

  int32_t pop_r(uint8_t type);              // pop to a 32bit register
  void pop_r(int32_t index, uint8_t type);  // pop to the given 32bit register
  void push_r(int32_t index);               // push a register

  void restore_state(wasm_jit_ctx_t* from, wasm_jit_ctx_t* to);

  typedef void (CodeGenerator::*oo_op_t)(const Xbyak::Operand&,
                                         const Xbyak::Operand&);
  typedef void (CodeGenerator::*or8_op_t)(const Xbyak::Operand&,
                                          const Xbyak::Reg8&);
  typedef void (CodeGenerator::*o_op_t)(const Xbyak::Operand&);
  typedef void (CodeGenerator::*xo_op_t)(const Xbyak::Xmm&,
                                         const Xbyak::Operand&);

  int32_t pop_binop_r32_r32(oo_op_t binop);
  int32_t pop_binop_xmm_xmm(xo_op_t binop);

  void rbp_load_r(int32_t index, int32_t r, uint8_t type);
  void rbp_save_r(int32_t index, int32_t r);

  int32_t alloc_special(wasm_jit_type_t t,
                        size_t off);  // load global base to a register
  int32_t alloc_global_base();        // load global base to a register
  int32_t alloc_memory_base();        // load memory base to a register
  int32_t alloc_memory_size();        // load memory size to a register

  void emit_lea_r_rip(int32_t r, size_t dest_offset);
  void emit_mov_r_rip(int32_t r, size_t dest_offset);
  void emit_call_internal(wasm_func_decl_t* sig, int is_direct, int index);
  void emit_pop_result();

  void emit_unreachable();
  void emit_nop();
  void emit_drop();
  void emit_select();
  void emit_local_get();
  void emit_local_set();
  void emit_local_tee();
  void emit_global_get();
  void emit_global_set();
  void emit_i32_load();
  void emit_f64_load();
  void emit_i32_store();
  void emit_f64_store();
  void emit_i32_const();
  void emit_f64_const();
  void emit_i32_numeric();
  void emit_i32_shift();
  void emit_i32_convert();
  void emit_f64_compute();
  void emit_call();
  void emit_call_indirect();
  void emit_return();
  void emit_block();
  void emit_end();
  void emit_br();
  void emit_br_if();
  void emit_br_table();
};
