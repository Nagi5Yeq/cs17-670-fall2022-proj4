#include <cmath>
#include <iostream>

#include "weerun.hpp"
#include "weerun_jit.hpp"

#define INVALID_INST \
  { 0, 0, "illegal", nullptr, nullptr }

static void run_unreachable(wasm_instance_t* ins, byte opcode);
static void run_nop(wasm_instance_t* ins, byte opcode);
static void run_drop(wasm_instance_t* ins, byte opcode);
static void run_select(wasm_instance_t* ins, byte opcode);
static void run_local_get(wasm_instance_t* ins, byte opcode);
static void run_local_set(wasm_instance_t* ins, byte opcode);
static void run_local_tee(wasm_instance_t* ins, byte opcode);
static void run_global_get(wasm_instance_t* ins, byte opcode);
static void run_global_set(wasm_instance_t* ins, byte opcode);

#if 0
static void run_tabel_get(wasm_instance_t* ins, byte opcode);
static void run_tabel_set(wasm_instance_t* ins, byte opcode);
#endif

static void run_i32_load(wasm_instance_t* ins, byte opcode);
static void run_f64_load(wasm_instance_t* ins, byte opcode);
static void run_i32_load8_u(wasm_instance_t* ins, byte opcode);
static void run_i32_load8_s(wasm_instance_t* ins, byte opcode);
static void run_i32_load16_u(wasm_instance_t* ins, byte opcode);
static void run_i32_load16_s(wasm_instance_t* ins, byte opcode);
static void run_i32_store(wasm_instance_t* ins, byte opcode);
static void run_f64_store(wasm_instance_t* ins, byte opcode);
static void run_i32_store8(wasm_instance_t* ins, byte opcode);
static void run_i32_store16(wasm_instance_t* ins, byte opcode);
static void run_i32_const(wasm_instance_t* ins, byte opcode);
static void run_f64_const(wasm_instance_t* ins, byte opcode);
static void run_i32_numeric(wasm_instance_t* ins, byte opcode);
static void run_f64_compare(wasm_instance_t* ins, byte opcode);
static void run_f64_compute(wasm_instance_t* ins, byte opcode);
static void run_f64_convert(wasm_instance_t* ins, byte opcode);
static void run_call(wasm_instance_t* ins, byte opcode);
static void run_call_indirect(wasm_instance_t* ins, byte opcode);
static void run_return(wasm_instance_t* ins, byte opcode);
static void run_block(wasm_instance_t* ins, byte opcode);
static void run_end(wasm_instance_t* ins, byte opcode);
static void run_br(wasm_instance_t* ins, byte opcode);
static void run_br_table(wasm_instance_t* ins, byte opcode);

const struct wasm_inst_desc_t g_inst_desc[] = {
    {WASM_OP_UNREACHABLE, WASM_INST_TYPE_NOARG, "unreachable", run_unreachable,
     &wasm_jit_t::emit_unreachable},
    {WASM_OP_NOP, WASM_INST_TYPE_NOARG, "nop", run_nop, &wasm_jit_t::emit_nop},
    {WASM_OP_BLOCK, WASM_INST_TYPE_BLOCK, "block", run_block,
     &wasm_jit_t::emit_block},
    {WASM_OP_LOOP, WASM_INST_TYPE_BLOCK, "loop", run_block,
     &wasm_jit_t::emit_block},
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    {WASM_OP_END, WASM_INST_TYPE_NOARG, "end", run_end, &wasm_jit_t::emit_end},
    {WASM_OP_BR, WASM_INST_TYPE_I32, "br", run_br, &wasm_jit_t::emit_br},
    {WASM_OP_BR_IF, WASM_INST_TYPE_I32, "br_if", run_br,
     &wasm_jit_t::emit_br_if},
    {WASM_OP_BR_TABLE, WASM_INST_TYPE_BRTABLE, "br_tale", run_br_table,
     &wasm_jit_t::emit_br_table},
    {WASM_OP_RETURN, WASM_INST_TYPE_NOARG, "return", run_return,
     &wasm_jit_t::emit_return},
    {WASM_OP_CALL, WASM_INST_TYPE_I32, "call", run_call,
     &wasm_jit_t::emit_call},
    {WASM_OP_CALL_INDIRECT, WASM_INST_TYPE_I32_I32, "call_indirect",
     run_call_indirect, &wasm_jit_t::emit_call_indirect},
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    {WASM_OP_DROP, WASM_INST_TYPE_NOARG, "drop", run_drop,
     &wasm_jit_t::emit_drop},
    {WASM_OP_SELECT, WASM_INST_TYPE_NOARG, "select", run_select,
     &wasm_jit_t::emit_select},
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    {WASM_OP_LOCAL_GET, WASM_INST_TYPE_I32, "local.get", run_local_get,
     &wasm_jit_t::emit_local_get},
    {WASM_OP_LOCAL_SET, WASM_INST_TYPE_I32, "local.set", run_local_set,
     &wasm_jit_t::emit_local_set},
    {WASM_OP_LOCAL_TEE, WASM_INST_TYPE_I32, "local.tee", run_local_tee,
     &wasm_jit_t::emit_local_tee},
    {WASM_OP_GLOBAL_GET, WASM_INST_TYPE_I32, "global.get", run_global_get,
     &wasm_jit_t::emit_global_get},
    {WASM_OP_GLOBAL_SET, WASM_INST_TYPE_I32, "global.set", run_global_set,
     &wasm_jit_t::emit_global_set},
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    {WASM_OP_I32_LOAD, WASM_INST_TYPE_MEMARG, "i32.load", run_i32_load,
     &wasm_jit_t::emit_i32_load},
    INVALID_INST,
    INVALID_INST,
    {WASM_OP_F64_LOAD, WASM_INST_TYPE_MEMARG, "f64.load", run_f64_load,
     &wasm_jit_t::emit_f64_load},
    {WASM_OP_I32_LOAD8_S, WASM_INST_TYPE_MEMARG, "i32.load8_s", run_i32_load8_s,
     &wasm_jit_t::emit_i32_load},
    {WASM_OP_I32_LOAD8_U, WASM_INST_TYPE_MEMARG, "i32.load8_u", run_i32_load8_u,
     &wasm_jit_t::emit_i32_load},
    {WASM_OP_I32_LOAD16_S, WASM_INST_TYPE_MEMARG, "i32.load16_s",
     run_i32_load16_s, &wasm_jit_t::emit_i32_load},
    {WASM_OP_I32_LOAD16_U, WASM_INST_TYPE_MEMARG, "i32.load16_u",
     run_i32_load16_u, &wasm_jit_t::emit_i32_load},
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    {WASM_OP_I32_STORE, WASM_INST_TYPE_MEMARG, "i32.store", run_i32_store,
     &wasm_jit_t::emit_i64_store},
    {WASM_OP_I32_STORE, WASM_INST_TYPE_MEMARG, "i64.store", run_i32_store,
     &wasm_jit_t::emit_i64_store},
    INVALID_INST,
    {WASM_OP_F64_STORE, WASM_INST_TYPE_MEMARG, "f64.store", run_f64_store,
     &wasm_jit_t::emit_f64_store},
    {WASM_OP_I32_STORE8, WASM_INST_TYPE_MEMARG, "i32.store8", run_i32_store8,
     &wasm_jit_t::emit_i64_store},
    {WASM_OP_I32_STORE16, WASM_INST_TYPE_MEMARG, "i32.store16", run_i32_store16,
     &wasm_jit_t::emit_i64_store},
    {WASM_OP_I32_STORE16, WASM_INST_TYPE_MEMARG, "i64.store8", run_i32_store16,
     &wasm_jit_t::emit_i64_store},
    {WASM_OP_I32_STORE16, WASM_INST_TYPE_MEMARG, "i64.store16", run_i32_store16,
     &wasm_jit_t::emit_i64_store},
    {WASM_OP_I32_STORE16, WASM_INST_TYPE_MEMARG, "i64.store32", run_i32_store16,
     &wasm_jit_t::emit_i64_store},
    INVALID_INST,
    INVALID_INST,
    {WASM_OP_I32_CONST, WASM_INST_TYPE_S32, "i32.const", run_i32_const,
     &wasm_jit_t::emit_i32_const},
    {WASM_OP_I32_CONST, WASM_INST_TYPE_S32, "i64.const", run_i32_const,
     &wasm_jit_t::emit_i64_const},
    INVALID_INST,
    {WASM_OP_F64_CONST, WASM_INST_TYPE_F64, "f64.const", run_f64_const,
     &wasm_jit_t::emit_f64_const},
    {WASM_OP_I32_EQZ, WASM_INST_TYPE_NOARG, "i32.eqz", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_EQ, WASM_INST_TYPE_NOARG, "i32.eq", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_NE, WASM_INST_TYPE_NOARG, "i32.ne", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_LT_S, WASM_INST_TYPE_NOARG, "i32.lt_s", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_LT_U, WASM_INST_TYPE_NOARG, "i32.lt_u", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_GT_S, WASM_INST_TYPE_NOARG, "i32.gt_s", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_GT_U, WASM_INST_TYPE_NOARG, "i32.gt_u", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_LE_S, WASM_INST_TYPE_NOARG, "i32.le_s", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_LE_U, WASM_INST_TYPE_NOARG, "i32.le_u", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_GE_S, WASM_INST_TYPE_NOARG, "i32.ge_s", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_GE_U, WASM_INST_TYPE_NOARG, "i32.ge_u", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    {WASM_OP_F64_EQ, WASM_INST_TYPE_NOARG, "f64.eq", run_f64_compare,
     &wasm_jit_t::emit_f64_compute},
    {WASM_OP_F64_NE, WASM_INST_TYPE_NOARG, "f64.ne", run_f64_compare,
     &wasm_jit_t::emit_f64_compute},
    {WASM_OP_F64_LT, WASM_INST_TYPE_NOARG, "f64.lt", run_f64_compare,
     &wasm_jit_t::emit_f64_compute},
    {WASM_OP_F64_GT, WASM_INST_TYPE_NOARG, "f64.gt", run_f64_compare,
     &wasm_jit_t::emit_f64_compute},
    {WASM_OP_F64_LE, WASM_INST_TYPE_NOARG, "f64.le", run_f64_compare,
     &wasm_jit_t::emit_f64_compute},
    {WASM_OP_F64_GE, WASM_INST_TYPE_NOARG, "f64.ge", run_f64_compare,
     &wasm_jit_t::emit_f64_compute},
    {WASM_OP_I32_CLZ, WASM_INST_TYPE_NOARG, "i32.clz", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_CTZ, WASM_INST_TYPE_NOARG, "i32.ctz", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_POPCNT, WASM_INST_TYPE_NOARG, "i32.popcnt", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_ADD, WASM_INST_TYPE_NOARG, "i32.add", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_SUB, WASM_INST_TYPE_NOARG, "i32.sub", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_MUL, WASM_INST_TYPE_NOARG, "i32.mul", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_DIV_S, WASM_INST_TYPE_NOARG, "i32.div_s", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_DIV_U, WASM_INST_TYPE_NOARG, "i32.div_u", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_REM_S, WASM_INST_TYPE_NOARG, "i32.rem_s", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_REM_U, WASM_INST_TYPE_NOARG, "i32.rem_u", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_AND, WASM_INST_TYPE_NOARG, "i32.and", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_OR, WASM_INST_TYPE_NOARG, "i32.or", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_XOR, WASM_INST_TYPE_NOARG, "i32.xor", run_i32_numeric,
     &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_SHL, WASM_INST_TYPE_NOARG, "i32.shl", run_i32_numeric,
     &wasm_jit_t::emit_i32_shift},
    {WASM_OP_I32_SHR_S, WASM_INST_TYPE_NOARG, "i32.shr_s", run_i32_numeric,
     &wasm_jit_t::emit_i32_shift},
    {WASM_OP_I32_SHR_U, WASM_INST_TYPE_NOARG, "i32.shr_u", run_i32_numeric,
     &wasm_jit_t::emit_i32_shift},
    {WASM_OP_I32_ROTL, WASM_INST_TYPE_NOARG, "i32.rotl", run_i32_numeric,
     &wasm_jit_t::emit_i32_shift},
    {WASM_OP_I32_ROTR, WASM_INST_TYPE_NOARG, "i32.rotr", run_i32_numeric,
     &wasm_jit_t::emit_i32_shift},
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    {WASM_OP_F64_ADD, WASM_INST_TYPE_NOARG, "f64.add", run_f64_compute,
     &wasm_jit_t::emit_f64_compute},
    {WASM_OP_F64_SUB, WASM_INST_TYPE_NOARG, "f64.sub", run_f64_compute,
     &wasm_jit_t::emit_f64_compute},
    {WASM_OP_F64_MUL, WASM_INST_TYPE_NOARG, "f64.mul", run_f64_compute,
     &wasm_jit_t::emit_f64_compute},
    {WASM_OP_F64_DIV, WASM_INST_TYPE_NOARG, "f64.div", run_f64_compute,
     &wasm_jit_t::emit_f64_compute},
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    {WASM_OP_I32_TRUNC_F64_S, WASM_INST_TYPE_NOARG, "i32.trunc_f64_s",
     run_i32_numeric, &wasm_jit_t::emit_i32_convert},
    {WASM_OP_I32_TRUNC_F64_U, WASM_INST_TYPE_NOARG, "i32.trunc_f64_u",
     run_i32_numeric, &wasm_jit_t::emit_i32_convert},
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    {WASM_OP_F64_CONVERT_I32_S, WASM_INST_TYPE_NOARG, "f64.convert_i32_s",
     run_f64_convert, &wasm_jit_t::emit_f64_compute},
    {WASM_OP_F64_CONVERT_I32_U, WASM_INST_TYPE_NOARG, "f64.convert_i32_u",
     run_f64_convert, &wasm_jit_t::emit_f64_compute},
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    INVALID_INST,
    {WASM_OP_I32_EXTEND8_S, WASM_INST_TYPE_NOARG, "i32.extend8_s",
     run_i32_numeric, &wasm_jit_t::emit_i32_numeric},
    {WASM_OP_I32_EXTEND16_S, WASM_INST_TYPE_NOARG, "i32.extend16_s",
     run_i32_numeric, &wasm_jit_t::emit_i32_numeric},
};

static bool is_zero(wasm_value_t v, uint8_t type) {
  switch (type) {
    case WASM_TYPE_I32:
      return v.i32 == 0;
    case WASM_TYPE_F64:
      return v.f64 == 0;
    default:
      break;
  }
  return v.ref == nullptr;
}

// fancy ops
static inline uint32_t i32_clz(uint32_t x) {
  uint32_t zeros = 0;
  for (int i = 31; i >= 0; i--) {
    if ((x & (1 << i)) == 0) {
      zeros++;
    } else {
      break;
    }
  }
  return zeros;
}

static inline uint32_t i32_ctz(uint32_t x) {
  uint32_t zeros = 0;
  for (int i = 0; i < 32; i++) {
    if ((x & (1 << i)) == 0) {
      zeros++;
    } else {
      break;
    }
  }
  return zeros;
}

static inline uint32_t i32_popcnt(uint32_t x) {
  uint32_t ones = 0;
  for (int i = 31; i >= 0; i--) {
    if ((x & (1 << i)) != 0) {
      ones++;
    }
  }
  return ones;
}

static inline uint32_t i32_rotl(uint32_t x, uint32_t n) {
  return (x << n) | (x >> (32 - n));
}

static inline uint32_t i32_rotr(uint32_t x, uint32_t n) {
  return (x >> n) | (x << (32 - n));
}

static inline std::pair<bool, int32_t> i32_div_s(int32_t a, int32_t b) {
  if (b == 0) {
    return {false, 0};
  }
  if (a == (int32_t)0x80000000 && b == (int32_t)0xffffffff) {
    return {false, 0};
  }
  return {true, a / b};
}

static inline std::pair<bool, int32_t> i32_rem_s(int32_t a, int32_t b) {
  if (b == 0) {
    return {false, 0};
  }
  if (a == (int32_t)0x80000000 && b == (int32_t)0xffffffff) {
    return {true, 0};
  }
  return {true, a % b};
}

static inline std::pair<bool, uint32_t> i32_div_u(uint32_t a, uint32_t b) {
  if (b == 0) {
    return {false, 0};
  }
  return {true, a / b};
}

static inline std::pair<bool, uint32_t> i32_rem_u(uint32_t a, uint32_t b) {
  if (b == 0) {
    return {false, 0};
  }
  return {true, a % b};
}

static inline std::pair<bool, int32_t> i32_trunc_f64_s(double x) {
  int type = std::fpclassify(x);
  if (type == FP_NAN || type == FP_INFINITE) {
    return {false, 0};
  }
  if (x <= -2147483649.0 || x >= 2147483648.0) {
    return {false, 0};
  }
  return {true, x};
}

static inline std::pair<bool, uint32_t> i32_trunc_f64_u(double x) {
  int type = std::fpclassify(x);
  if (type == FP_NAN || type == FP_INFINITE) {
    return {false, 0};
  }
  if (x <= -1.0 || x >= 4294967296.0) {
    return {false, 0};
  }
  return {true, x};
}

template <typename T>
static inline bool check_access(wasm_instance_t* ins,
                                uint32_t offset,
                                uint32_t addend) {
  uint32_t size = ins->memory_size_;
  return offset <= size && addend <= size &&
         offset + addend + sizeof(T) <= size;
}

static void run_unreachable(wasm_instance_t* ins, byte opcode) {
  ins->trap(WASM_TRAP_REASON_UNREACHABLE);
}

static void run_nop(wasm_instance_t* ins, byte opcode) {}

static void run_drop(wasm_instance_t* ins, byte opcode) {
  ins->pop_value();
}

static void run_select(wasm_instance_t* ins, byte opcode) {
  wasm_value_t c = ins->pop_value();
  wasm_value_t b = ins->pop_value();
  wasm_value_t a = ins->pop_value();
  if (is_zero(c, WASM_TYPE_I32) == false) {
    ins->push_value(a);
  } else {
    ins->push_value(b);
  }
}

static void run_local_get(wasm_instance_t* ins, byte opcode) {
  uint32_t idx = ins->read_u32leb();
  ins->push_value(ins->cur_frame_->locals_[idx]);
}

static void run_local_set(wasm_instance_t* ins, byte opcode) {
  uint32_t idx = ins->read_u32leb();
  ins->cur_frame_->locals_[idx] = ins->pop_value();
}

static void run_local_tee(wasm_instance_t* ins, byte opcode) {
  uint32_t idx = ins->read_u32leb();
  wasm_value_t v = ins->value_stack_.back();
  ins->cur_frame_->locals_[idx] = v;
}

static void run_global_get(wasm_instance_t* ins, byte opcode) {
  uint32_t idx = ins->read_u32leb();
  ins->push_value(ins->globals_[idx]);
}

static void run_global_set(wasm_instance_t* ins, byte opcode) {
  uint32_t idx = ins->read_u32leb();
  ins->globals_[idx] = ins->pop_value();
}

#if 0
static void run_tabel_get(wasm_instance_t* ins, byte opcode) {
  uint32_t idx = ins->read_u32leb();
  wasm_value_t v;
  v.ref = (void*)ins->table_[idx];
  ins->push_value(v);
}

static void run_tabel_set(wasm_instance_t* ins, byte opcode) {
  uint32_t idx = ins->read_u32leb();
  ins->table_[idx] = (wasm_func_t*)ins->pop_value().ref;
}
#endif

static void run_i32_load(wasm_instance_t* ins, byte opcode) {
  uint32_t addend = ins->pop_value().i32;
  ins->read_u32leb();
  uint32_t offset = ins->read_u32leb();
  if (check_access<uint32_t>(ins, offset, addend) == false) {
    ins->trap(WASM_TRAP_REASON_OOB);
    return;
  }
  byte* base = ins->memory_.data();
  wasm_value_t v;
  v.i32 = *(uint32_t*)(base + (offset + addend));
  ins->push_value(v);
}

static void run_f64_load(wasm_instance_t* ins, byte opcode) {
  uint32_t addend = ins->pop_value().i32;
  ins->read_u32leb();
  uint32_t offset = ins->read_u32leb();
  if (check_access<double>(ins, offset, addend) == false) {
    ins->trap(WASM_TRAP_REASON_OOB);
    return;
  }
  byte* base = ins->memory_.data();
  wasm_value_t v;
  v.f64 = *(double*)(base + (offset + addend));
  ins->push_value(v);
}

static void run_i32_load8_u(wasm_instance_t* ins, byte opcode) {
  uint32_t addend = ins->pop_value().i32;
  ins->read_u32leb();
  uint32_t offset = ins->read_u32leb();
  if (check_access<uint8_t>(ins, offset, addend) == false) {
    ins->trap(WASM_TRAP_REASON_OOB);
    return;
  }
  byte* base = ins->memory_.data();
  uint8_t u8 = *(uint8_t*)(base + (offset + addend));
  wasm_value_t v;
  v.i32 = (uint32_t)u8;
  ins->push_value(v);
}

static void run_i32_load8_s(wasm_instance_t* ins, byte opcode) {
  uint32_t addend = ins->pop_value().i32;
  ins->read_u32leb();
  uint32_t offset = ins->read_u32leb();
  if (check_access<int8_t>(ins, offset, addend) == false) {
    ins->trap(WASM_TRAP_REASON_OOB);
    return;
  }
  byte* base = ins->memory_.data();
  int8_t i8 = *(int8_t*)(base + (offset + addend));
  wasm_value_t v;
  v.i32 = (uint32_t)(int32_t)i8;
  ins->push_value(v);
}

static void run_i32_load16_u(wasm_instance_t* ins, byte opcode) {
  uint32_t addend = ins->pop_value().i32;
  ins->read_u32leb();
  uint32_t offset = ins->read_u32leb();
  if (check_access<uint16_t>(ins, offset, addend) == false) {
    ins->trap(WASM_TRAP_REASON_OOB);
    return;
  }
  byte* base = ins->memory_.data();
  uint16_t u16 = *(uint16_t*)(base + (offset + addend));
  wasm_value_t v;
  v.i32 = (uint32_t)u16;
  ins->push_value(v);
}

static void run_i32_load16_s(wasm_instance_t* ins, byte opcode) {
  uint32_t addend = ins->pop_value().i32;
  ins->read_u32leb();
  uint32_t offset = ins->read_u32leb();
  if (check_access<int16_t>(ins, offset, addend) == false) {
    ins->trap(WASM_TRAP_REASON_OOB);
    return;
  }
  byte* base = ins->memory_.data();
  int16_t i16 = *(int16_t*)(base + (offset + addend));
  wasm_value_t v;
  v.i32 = (uint32_t)(int32_t)i16;
  ins->push_value(v);
}

static void run_i32_store(wasm_instance_t* ins, byte opcode) {
  wasm_value_t v = ins->pop_value();
  uint32_t addend = ins->pop_value().i32;
  ins->read_u32leb();
  uint32_t offset = ins->read_u32leb();
  if (check_access<uint32_t>(ins, offset, addend) == false) {
    ins->trap(WASM_TRAP_REASON_OOB);
    return;
  }
  byte* base = ins->memory_.data();
  *(uint32_t*)(base + (offset + addend)) = v.i32;
}

static void run_f64_store(wasm_instance_t* ins, byte opcode) {
  wasm_value_t v = ins->pop_value();
  uint32_t addend = ins->pop_value().i32;
  ins->read_u32leb();
  uint32_t offset = ins->read_u32leb();
  if (check_access<double>(ins, offset, addend) == false) {
    ins->trap(WASM_TRAP_REASON_OOB);
    return;
  }
  byte* base = ins->memory_.data();
  *(double*)(base + (offset + addend)) = v.f64;
}

static void run_i32_store8(wasm_instance_t* ins, byte opcode) {
  wasm_value_t v = ins->pop_value();
  uint32_t addend = ins->pop_value().i32;
  ins->read_u32leb();
  uint32_t offset = ins->read_u32leb();
  if (check_access<uint8_t>(ins, offset, addend) == false) {
    ins->trap(WASM_TRAP_REASON_OOB);
    return;
  }
  byte* base = ins->memory_.data();
  *(uint8_t*)(base + (offset + addend)) = (uint8_t)v.i32;
}

static void run_i32_store16(wasm_instance_t* ins, byte opcode) {
  wasm_value_t v = ins->pop_value();
  uint32_t addend = ins->pop_value().i32;
  ins->read_u32leb();
  uint32_t offset = ins->read_u32leb();
  if (check_access<uint16_t>(ins, offset, addend) == false) {
    ins->trap(WASM_TRAP_REASON_OOB);
    return;
  }
  byte* base = ins->memory_.data();
  *(uint16_t*)(base + (offset + addend)) = (uint16_t)v.i32;
}

static void run_i32_const(wasm_instance_t* ins, byte opcode) {
  uint32_t x = ins->read_i32leb();
  wasm_value_t v;
  v.i32 = x;
  ins->push_value(v);
}

static void run_f64_const(wasm_instance_t* ins, byte opcode) {
  double x = ins->read_double();
  wasm_value_t v;
  v.f64 = x;
  ins->push_value(v);
}

static void run_i32_numeric(wasm_instance_t* ins, byte opcode) {
  wasm_value_t a;
  wasm_value_t b;
  wasm_value_t result;

  b = ins->pop_value();
  switch (opcode) {
    case WASM_OP_I32_EQZ:
      result.i32 = (b.i32 == 0);
      break;
    case WASM_OP_I32_EQ:
      a = ins->pop_value();
      result.i32 = (a.i32 == b.i32);
      break;
    case WASM_OP_I32_NE:
      a = ins->pop_value();
      result.i32 = (a.i32 != b.i32);
      break;
    case WASM_OP_I32_LT_S:
      a = ins->pop_value();
      result.i32 = ((int32_t)a.i32 < (int32_t)b.i32);
      break;
    case WASM_OP_I32_LT_U:
      a = ins->pop_value();
      result.i32 = (a.i32 < b.i32);
      break;
    case WASM_OP_I32_GT_S:
      a = ins->pop_value();
      result.i32 = ((int32_t)a.i32 > (int32_t)b.i32);
      break;
    case WASM_OP_I32_GT_U:
      a = ins->pop_value();
      result.i32 = (a.i32 > b.i32);
      break;
    case WASM_OP_I32_LE_S:
      a = ins->pop_value();
      result.i32 = ((int32_t)a.i32 <= (int32_t)b.i32);
      break;
    case WASM_OP_I32_LE_U:
      a = ins->pop_value();
      result.i32 = (a.i32 <= b.i32);
      break;
    case WASM_OP_I32_GE_S:
      a = ins->pop_value();
      result.i32 = ((int32_t)a.i32 >= (int32_t)b.i32);
      break;
    case WASM_OP_I32_GE_U:
      a = ins->pop_value();
      result.i32 = (a.i32 >= b.i32);
      break;
    case WASM_OP_I32_CLZ:
      result.i32 = i32_clz(b.i32);
      break;
    case WASM_OP_I32_CTZ:
      result.i32 = i32_ctz(b.i32);
      break;
    case WASM_OP_I32_POPCNT:
      result.i32 = i32_popcnt(b.i32);
      break;
    case WASM_OP_I32_ADD:
      a = ins->pop_value();
      result.i32 = (a.i32 + b.i32);
      break;
    case WASM_OP_I32_SUB:
      a = ins->pop_value();
      result.i32 = (a.i32 - b.i32);
      break;
    case WASM_OP_I32_MUL:
      a = ins->pop_value();
      result.i32 = (a.i32 * b.i32);
      break;
    case WASM_OP_I32_DIV_S: {
      a = ins->pop_value();
      auto r = i32_div_s((int32_t)a.i32, (int32_t)b.i32);
      if (r.first == false) {
        ins->trap(WASM_TRAP_REASON_SIGFPE);
        return;
      }
      result.i32 = r.second;
      break;
    }
    case WASM_OP_I32_DIV_U: {
      a = ins->pop_value();
      auto r = i32_div_u((int32_t)a.i32, (int32_t)b.i32);
      if (r.first == false) {
        ins->trap(WASM_TRAP_REASON_SIGFPE);
        return;
      }
      result.i32 = r.second;
      break;
    }
    case WASM_OP_I32_REM_S: {
      a = ins->pop_value();
      auto r = i32_rem_s((int32_t)a.i32, (int32_t)b.i32);
      if (r.first == false) {
        ins->trap(WASM_TRAP_REASON_SIGFPE);
        return;
      }
      result.i32 = r.second;
      break;
    }
    case WASM_OP_I32_REM_U: {
      a = ins->pop_value();
      auto r = i32_rem_u((int32_t)a.i32, (int32_t)b.i32);
      if (r.first == false) {
        ins->trap(WASM_TRAP_REASON_SIGFPE);
        return;
      }
      result.i32 = r.second;
      break;
    }
    case WASM_OP_I32_AND:
      a = ins->pop_value();
      result.i32 = (a.i32 & b.i32);
      break;
    case WASM_OP_I32_OR:
      a = ins->pop_value();
      result.i32 = (a.i32 | b.i32);
      break;
    case WASM_OP_I32_XOR:
      a = ins->pop_value();
      result.i32 = (a.i32 ^ b.i32);
      break;
    case WASM_OP_I32_SHL:
      a = ins->pop_value();
      result.i32 = (a.i32 << b.i32);
      break;
    case WASM_OP_I32_SHR_S:
      a = ins->pop_value();
      result.i32 = ((int32_t)a.i32 >> b.i32);
      break;
    case WASM_OP_I32_SHR_U:
      a = ins->pop_value();
      result.i32 = (a.i32 >> b.i32);
      break;
    case WASM_OP_I32_ROTL:
      a = ins->pop_value();
      result.i32 = i32_rotl(a.i32, b.i32);
      break;
    case WASM_OP_I32_ROTR:
      a = ins->pop_value();
      result.i32 = i32_rotr(a.i32, b.i32);
      break;
    case WASM_OP_I32_TRUNC_F64_S: {
      auto r = i32_trunc_f64_s(b.f64);
      if (r.first == false) {
        ins->trap(WASM_TRAP_REASON_SIGFPE);
        return;
      }
      result.i32 = r.second;
      break;
    }
    case WASM_OP_I32_TRUNC_F64_U: {
      auto r = i32_trunc_f64_u(b.f64);
      if (r.first == false) {
        ins->trap(WASM_TRAP_REASON_SIGFPE);
        return;
      }
      result.i32 = r.second;
      break;
    }
    case WASM_OP_I32_EXTEND8_S:
      result.i32 = (int32_t)(int8_t)b.i32;
      break;
    case WASM_OP_I32_EXTEND16_S:
      result.i32 = (int32_t)(int16_t)b.i32;
      break;
  }
  ins->push_value(result);
}

static void run_f64_compare(wasm_instance_t* ins, byte opcode) {
  wasm_value_t a;
  wasm_value_t b;
  wasm_value_t result;

  b = ins->pop_value();
  a = ins->pop_value();
  switch (opcode) {
    case WASM_OP_F64_LT:
      result.i32 = (a.f64 < b.f64);
      break;
    case WASM_OP_F64_GT:
      result.i32 = (a.f64 > b.f64);
      break;
    case WASM_OP_F64_LE:
      result.i32 = (a.f64 <= b.f64);
      break;
    case WASM_OP_F64_GE:
      result.i32 = (a.f64 >= b.f64);
      break;
    case WASM_OP_F64_EQ:
      result.i32 = (a.f64 == b.f64);
      break;
    case WASM_OP_F64_NE:
      result.i32 = (a.f64 != b.f64);
      break;
  }
  ins->push_value(result);
}

static void run_f64_compute(wasm_instance_t* ins, byte opcode) {
  wasm_value_t a;
  wasm_value_t b;
  wasm_value_t result;

  b = ins->pop_value();
  a = ins->pop_value();
  switch (opcode) {
    case WASM_OP_F64_ADD:
      result.f64 = (a.f64 + b.f64);
      break;
    case WASM_OP_F64_SUB:
      result.f64 = (a.f64 - b.f64);
      break;
    case WASM_OP_F64_MUL:
      result.f64 = (a.f64 * b.f64);
      break;
    case WASM_OP_F64_DIV:
      result.f64 = (a.f64 / b.f64);
      break;
  }
  ins->push_value(result);
}

static void run_f64_convert(wasm_instance_t* ins, byte opcode) {
  wasm_value_t b;
  wasm_value_t result;

  b = ins->pop_value();
  switch (opcode) {
    case WASM_OP_F64_CONVERT_I32_S:
      result.f64 = (int32_t)b.i32;
      break;
    case WASM_OP_F64_CONVERT_I32_U:
      result.f64 = b.i32;
      break;
  }
  ins->push_value(result);
}

static void run_call(wasm_instance_t* ins, byte opcode) {
  uint32_t funcidx = ins->read_u32leb();
  ins->push_frame(&ins->module_->funcs_[funcidx]);
}

static void run_call_indirect(wasm_instance_t* ins, byte opcode) {
  ins->read_u32leb();
  uint32_t typeidx = ins->read_u32leb();
  wasm_value_t v = ins->pop_value();
  if (v.i32 >= ins->table_.size() || typeidx >= ins->module_->sigs_.size()) {
    ins->trap(WASM_TRAP_REASON_OOB);
    return;
  }
  wasm_func_t* f = ins->table_[v.i32];
  if (f == nullptr) {
    ins->trap(WASM_TRAP_REASON_NULLFUNCREF);
    return;
  }
  if (typeidx != f->typeidx_) {
    ins->trap(WASM_TRAP_REASON_TYPEERR);
    return;
  }
  ins->push_frame(ins->table_[v.i32]);
}

static void run_return(wasm_instance_t* ins, byte opcode) {
  ins->pop_frame();
  if (ins->frames_.empty()) {
    ins->trap(WASM_TRAP_REASON_FUNCEND);
  }
}

static void run_block(wasm_instance_t* ins, byte opcode) {
  // read the blocktype byte and go
  ins->read_byte();
}

static void run_end(wasm_instance_t* ins, byte opcode) {
  // only the OP_END for the outermost function matters
  if (ins->cur_frame_->pc_.ptr == ins->cur_frame_->pc_.end) {
    ins->pop_frame();
    if (ins->frames_.empty()) {
      ins->trap(WASM_TRAP_REASON_FUNCEND);
    }
  }
}

static void run_br(wasm_instance_t* ins, byte opcode) {
  if (opcode == WASM_OP_BR_IF) {
    wasm_value_t v = ins->pop_value();
    if (is_zero(v, WASM_TYPE_I32) == true) {
      ins->cur_frame_->pc_.ptr += sizeof(uint32_t);
      return;
    }
  }
  uint32_t to_off = *(uint32_t*)(ins->cur_frame_->pc_.ptr);
  ins->branch(to_off);
}

static void run_br_table(wasm_instance_t* ins, byte opcode) {
  uint32_t num_cases = ins->read_u32leb();
  uint32_t* cases = (uint32_t*)(ins->cur_frame_->pc_.ptr);
  wasm_value_t v = ins->pop_value();
  uint32_t to_off;
  uint32_t index = std::min(v.i32, num_cases);
  to_off = cases[index];
  ins->branch(to_off);
}
