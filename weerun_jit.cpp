#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

#include "weerun_jit.hpp"

#define JIT_V(type, i) \
  (wasm_jit_val_t) { wasm_jit_type_t::type, (i) }

#define JIT_V_INVALID JIT_V(INVALID, 0)
#define JIT_V_CALLER JIT_V(CALLER_DATA, 0)
#define JIT_V_INTER JIT_V(INTERMEDIATE, 0)
#define JIT_V_LOCAL(i) JIT_V(LOCAL_VAL, (int32_t)(i))
#define JIT_V_STACK(i) JIT_V(STACK_VAL, (int32_t)(i))
#define JIT_V_GLOBAL_BASE JIT_V(GLOBAL_BASE, 0)
#define JIT_V_MEMORY_BASE JIT_V(MEMORY_BASE, 0)
#define JIT_V_MEMORY_SIZE JIT_V(MEMORY_SIZE, 0)

const static int32_t sysv_args[] = {reg::rdi, reg::rsi, reg::rdx,
                                    reg::rcx, reg::r8,  reg::r9};

inline Xbyak::Reg64 wasm_jit_t::to_xbyak_r64(int32_t index) {
  const static Xbyak::Reg64 xbyak_regs[] = {rax, rdx, rcx, rdi, rsi, r8,  r9,
                                            r10, r11, rbx, r12, r13, r14, r15};
  return xbyak_regs[index];
}

inline Xbyak::Reg32 wasm_jit_t::to_xbyak_r32(int32_t index) {
  const static Xbyak::Reg32 xbyak_regs[] = {eax,  edx,  ecx,  edi,  esi,
                                            r8d,  r9d,  r10d, r11d, ebx,
                                            r12d, r13d, r14d, r15d};
  return xbyak_regs[index];
}

inline Xbyak::Reg16 wasm_jit_t::to_xbyak_r16(int32_t index) {
  const static Xbyak::Reg16 xbyak_regs[] = {
      ax, dx, cx, di, si, r8w, r9w, r10w, r11w, bx, r12w, r13w, r14w, r15w};
  return xbyak_regs[index];
}

inline Xbyak::Reg8 wasm_jit_t::to_xbyak_r8(int32_t index) {
  const static Xbyak::Reg8 xbyak_regs[] = {
      al, dl, cl, dil, sil, r8b, r9b, r10b, r11b, bl, r12b, r13b, r14b, r15b};
  return xbyak_regs[index];
}

inline Xbyak::Xmm wasm_jit_t::to_xbyak_xmm(int32_t index) {
  const static Xbyak::Xmm xbyak_regs[] = {
      xmm0, xmm1, xmm2,  xmm3,  xmm4,  xmm5,  xmm6,  xmm7,
      xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15};
  return xbyak_regs[index - reg::xmm0];
}

// stack layout:
//        return address (-1)
// RBP --> saved rbp (0)
//         space for rbx,r12-r15 (1, +5)
//         locals (6, +num_locals)
//         wasm stack (6+num_locals, +max_stack)
// RSP -->

// function layout
// F1:
//  prologue (changed from nop after jitting body)
//  body (first generated)
// F1E:
//  epilogue (last generated)

// loop layout
//  OP_LOOP
// F1L1:
//  OP_WHATEVER
//  OP_BR -> L_1 (jmp to F1L1T0)
//  OP_WHATEVER
//  OP_BR -> L_1 (jmp to F1L2T1)
//  OP_END
//  (jmp to F1L1E)
// F1L1T0:
//  (restore context and jmp to F1L1)
// F1L2T1:
//  (restore context and jmp to F1L1)
// F1L1E:
//  OP_WHATEVER

// block layout
//  OP_BLOCK
//  OP_WHATEVER
//  OP_BR -> B_2 (F1B2 not generated, jmp to F1L2T0)
//  OP_WHATEVER
//  OP_BR -> B_2 (F1B2 not generated, jmp to F1L2T1)
//  OP_END
//  (jmp to F1L2E)
// F1L2T0:
//  (restore context and jmp to F1L2E)
// F1L2T1:
//  (restore context and jmp to F1L2E)
// F1L2E:
//  OP_WHATEVER

// br_table layout
//  OP_BR_TABLE
//  jmp [table_base + table_index*8]
// .c1:
//  jmp F1L1T1
// .c2
//  jmp F1L2T1
// .c3
//  jmp F1L1T1

// table content: .c1, .c2, .c3

void wasm_jit_ctx_t::init(wasm_jit_t* jit, wasm_func_t* f) {
  reg_ = {JIT_V_INVALID, JIT_V_INVALID, JIT_V_INVALID, JIT_V_INVALID,
          JIT_V_INVALID, JIT_V_INVALID, JIT_V_INVALID, JIT_V_INVALID,
          JIT_V_INVALID, JIT_V_CALLER,  JIT_V_CALLER,  JIT_V_CALLER,
          JIT_V_CALLER,  JIT_V_CALLER,  JIT_V_INVALID, JIT_V_INVALID,
          JIT_V_INVALID, JIT_V_INVALID, JIT_V_INVALID, JIT_V_INVALID,
          JIT_V_INVALID, JIT_V_INVALID, JIT_V_INVALID, JIT_V_INVALID,
          JIT_V_INVALID, JIT_V_INVALID, JIT_V_INVALID, JIT_V_INVALID,
          JIT_V_INVALID, JIT_V_INVALID};
  int num_args = f->sig_->args_.size();
  int num_locals = jit->local_types_.size();
  stack_valid_.resize(num_locals + 6, false);
  stack_loc_.clear();
  local_loc_.resize(num_locals);
  int num_r64 = 0;
  int num_f64 = 0;
  int num_stack_r64 = 0;
  int num_stack_f64 = 0;
  for (uint8_t arg : f->sig_->args_) {
    if (arg != WASM_TYPE_F64) {
      if (num_r64 < 6) {
        num_r64++;
      } else {
        num_stack_r64++;
      }
    } else {
      if (num_f64 < 8) {
        num_f64++;
      } else {
        num_stack_f64++;
      }
    }
  }
  int stack_arg_index = num_stack_r64 + num_stack_f64 + 1;
  for (int i = num_args - 1; i >= 0; i--) {
    uint8_t arg = f->sig_->args_[i];
    if (arg != WASM_TYPE_F64) {
      if (num_stack_r64 > 0) {
        local_loc_[i] = {reg::on_stack, i + 6};
        jit->mov(jit->rax, jit->qword[jit->rbp + (stack_arg_index * 0x8)]);
        jit->rbp_save_r(i + 6, reg::rax);
        stack_arg_index--;
        num_stack_r64--;
        local_loc_[i] = {reg::on_stack, i + 6};
      } else {
        num_r64--;
        local_loc_[i] = {reg::in_reg, sysv_args[num_r64]};
        reg_[sysv_args[num_r64]] = JIT_V_LOCAL(i);
      }
    } else {
      if (num_stack_f64 > 0) {
        local_loc_[i] = {reg::on_stack, i + 6};
        jit->mov(jit->rax, jit->qword[jit->rbp + (stack_arg_index * 0x8)]);
        jit->rbp_save_r(i + 6, reg::rax);
        stack_arg_index--;
        num_stack_f64--;
        local_loc_[i] = {reg::on_stack, i + 6};
      } else {
        num_f64--;
        local_loc_[i] = {reg::in_reg, reg::xmm0 + num_f64};
        reg_[reg::xmm0 + num_f64] = JIT_V_LOCAL(i);
      }
    }
  }
  bool r10_clear = false;
  bool xmm8_clear = false;
  for (int i = num_args; i < num_locals; i++) {
    local_loc_[i] = {reg::on_stack, i + 6};
    if (jit->local_types_[i] != WASM_TYPE_F64) {
      if (r10_clear == false) {
        jit->xor_(jit->r10, jit->r10);
        r10_clear = true;
      }
      jit->rbp_save_r(i + 6, reg::r10);
    } else {
      if (xmm8_clear == false) {
        jit->xorpd(jit->xmm8, jit->xmm8);
        xmm8_clear = true;
      }
      jit->rbp_save_r(i + 6, reg::xmm8);
    }
  }
  std::memset(reg_age_.data(), 0, 30 * sizeof(uint32_t));
  age_ = 0;
}

const static size_t memory_off = 0x0;
const static size_t global_off = 0x8;
const static size_t table_off = 0x10;
const static size_t memory_size_off = 0x18;
const static size_t table_size_off = 0x20;
const static size_t trap_off = 0x28;
const static size_t f64_const_off = 0x30;

void jit_trap() {
  std::cout << "!trap" << std::endl;
  std::exit(-1);
}

wasm_jit_t::wasm_jit_t(uint64_t code_size, bool check)
    : CodeGenerator(code_size), enable_check_(check) {
  size_t pos = getSize();
  endbr64();
  push(rbp);
  mov(rbp, rsp);
  push(rbx);
  push(r12);
  push(r13);
  push(r14);
  push(r15);
  sub(rsp, 0x12345678);
  max_prologue_size_ = getSize() - pos;
  reset();
}

typedef void (*run_void_t)(uint8_t* f,
                           uint64_t* reg,
                           uint64_t* stack,
                           size_t num_stack);
typedef uint64_t (*run_rax_t)(uint8_t* f,
                              uint64_t* reg,
                              uint64_t* stack,
                              size_t num_stack);
typedef double (*run_xmm_t)(uint8_t* f,
                            uint64_t* reg,
                            uint64_t* stack,
                            size_t num_stack);

// r_type 0:void 1:rax 2:xmm0
void wasm_jit_t::run_fn(uint32_t index, wasm_value_t* args, wasm_value_t* r) {
  wasm_func_decl_t* sig = mod_->funcs_[index].sig_;
  size_t num_args = sig->args_.size();
  uint8_t* arg_types = sig->args_.data();
  std::vector<uint64_t> stack;
  std::array<uint64_t, 14> reg;  // 6 r + 6 xmm
  size_t num_r64 = 0;
  size_t num_f64 = 0;
  for (size_t i = 0; i < num_args; i++) {
    if (arg_types[i] != WASM_TYPE_F64) {
      if (num_r64 < 6) {
        reg[num_r64] = args[i].i64;
        num_r64++;
      } else {
        stack.push_back(args[i].i64);
      }
    } else {
      if (num_f64 < 8) {
        reg[6 + num_f64] = args[i].i64;
        num_f64++;
      } else {
        stack.push_back(args[i].i64);
      }
    }
  }
  uint8_t* f = f_addr_[index];
  if (sig->ret_.empty() == true) {
    run_void_t run_void = (run_void_t)run_fn_;
    run_void(f, reg.data(), stack.data(), stack.size());
  } else if (sig->ret_[0] != WASM_TYPE_F64) {
    run_rax_t run_rax = (run_rax_t)run_fn_;
    r->i64 = run_rax(f, reg.data(), stack.data(), stack.size());
  } else {
    run_xmm_t run_xmm = (run_xmm_t)run_fn_;
    r->f64 = run_xmm(f, reg.data(), stack.data(), stack.size());
  }
}

void wasm_jit_t::compile_instance(wasm_instance_t* ins) {
  ins_ = ins;
  mod_ = ins->module_;
  f_addr_.resize(mod_->funcs_.size());

  dq((uint64_t)ins->memory_.data());
  dq((uint64_t)ins->globals_.data());
  dq((uint64_t)ins->jit_table_.data());
  dq((uint64_t)ins->memory_size_);
  dq((uint64_t)ins->jit_table_.size());
  dq((uint64_t)jit_trap);

  // f64 consts needed by i32.trunc, copied from GDB...
  dq(0x7fffffffffffffff);
  dq(0x0000000000000000);
  dq(0x7fefffffffffffff);
  dq(0x41e0000000000000);
  dq(0xc1e0000000200000);
  dq(0x41f0000000000000);
  dq(0xbff0000000000000);

  L("trap");
  push(rax);
  emit_mov_r_rip(reg::rax, trap_off);
  xchg(qword[rsp], rax);  // push address of trap() and preserve rax
  ret();                  // jump to trap()

  emit_runtime_functions();
  int num_funcs = mod_->funcs_.size();
  for (int i = mod_->num_import_funcs_; i < num_funcs; i++) {
    compile_function(i);
  }

  // fill jit table
  for (wasm_elem_decl_t& elem : mod_->elems_) {
    wasm_jit_table_entry_t* p = &ins_->jit_table_[elem.offset_];
    for (uint32_t funcidx : elem.funcidx_) {
      *p = {mod_->funcs_[funcidx].typeidx_, f_addr_[funcidx]};
      p++;
    }
  }
}

void wasm_jit_t::emit_runtime_functions() {
  align(8);
  size_t num_rt = mod_->num_import_funcs_;
  for (size_t i = 0; i < num_rt; i++) {
    wasm_func_t* f = &mod_->funcs_[i];
    f_addr_[i] = (uint8_t*)f->run_jit_;

    size_t offset = getSize();
    dq((uint64_t)f->run_jit_);
    L("F" + std::to_string(i));
    emit_lea_r_rip(reg::r10, offset);
    jmp(qword[r10]);
  }
  align(16);

  // rdi: f
  // rsi: reg
  // rdx: stack
  // rcx: num_stack
  run_fn_ = const_cast<uint8_t*>(getCurr());
  inLocalLabel();
  endbr64();
  push(rbp);
  mov(rbp, rsp);
  test(rcx, 0x1);
  je(".no_padding");
  sub(rsp, 0x8);
  L(".no_padding");
  lea(r8, qword[rdx + rcx * 8]);
  L(".push_stack");
  test(rcx, rcx);
  je(".no_stack");
  sub(r8, 0x8);
  mov(rax, qword[r8]);
  push(rax);
  dec(rcx);
  jmp(".push_stack");
  L(".no_stack");
  mov(r10, rdi);
  mov(rax, rsi);
  mov(rdi, qword[rax]);
  mov(rsi, qword[rax + 0x8]);
  mov(rdx, qword[rax + 0x10]);
  mov(rcx, qword[rax + 0x18]);
  mov(r8, qword[rax + 0x20]);
  mov(r9, qword[rax + 0x28]);
  movsd(xmm0, qword[rax + 0x30]);
  movsd(xmm1, qword[rax + 0x38]);
  movsd(xmm2, qword[rax + 0x40]);
  movsd(xmm3, qword[rax + 0x48]);
  movsd(xmm4, qword[rax + 0x50]);
  movsd(xmm5, qword[rax + 0x58]);
  movsd(xmm6, qword[rax + 0x60]);
  movsd(xmm7, qword[rax + 0x68]);
  call(r10);
  leave();
  ret();
  outLocalLabel();
}

void wasm_jit_t::compile_function(uint32_t index) {
  f_ = &mod_->funcs_[index];
  funcidx_ = index;
  int num_f_locals = 0;
  local_types_.clear();
  for (uint8_t t : f_->sig_->args_) {
    local_types_.push_back(t);
  }
  for (wasm_local_decl_t decl : f_->locals_) {
    for (int i = 0; i < (int)decl.count_; i++) {
      local_types_.push_back(decl.type_);
    }
    num_f_locals += decl.count_;
  }
  num_locals_ = f_->sig_->args_.size() + num_f_locals;

  rsp_sub_ = ((num_locals_ + 6) - 1) * 0x8;
  if (rsp_sub_ % 16 == 8) {
    rsp_sub_ += 8;
  }
  used_callee_ = {0, 0, 0, 0, 0};

  buf_.start = buf_.ptr = f_->code_.data();
  buf_.end = buf_.start + f_->code_.size();
  next_bi_ = 0;
  stop_ = 0;

  // save space for prologue
  size_t start_off = getSize();
  nop(max_prologue_size_, false);
  size_t body_off = getSize();
  uint8_t* body_pos = const_cast<uint8_t*>(getCurr());

  // generate function body
  ctx_.init(this, f_);
  block_stack_.push_back({0, next_bi_++});
  while (stop_ != 1) {
    opcode_ = read_u8(&buf_);
    const wasm_inst_desc_t* desc = &g_inst_desc[opcode_];
    if (desc->emit == nullptr) {
      __asm__("int3");
    }
    (this->*desc->emit)();
  }
  block_stack_.clear();
  size_t end_off = getSize();
  L("F" + std::to_string(funcidx_) + "E");

  // generate prologue
  setSize(start_off);
  endbr64();
  push(rbp);
  mov(rbp, rsp);
  for (int32_t i = 0; i < 5; i++) {
    if (used_callee_[i] != 0) {
      push(to_xbyak_r64(reg::rbx + i));
      rsp_sub_ -= 0x8;
    }
  }
  sub(rsp, rsp_sub_);
  size_t prologue_size = getSize() - start_off;
  size_t gap_size = max_prologue_size_ - prologue_size;
  for (size_t i = 0; i < prologue_size; i++) {
    body_pos--;
    *body_pos = *(body_pos - gap_size);
  }
  f_addr_[funcidx_] = body_pos;
  setSize(start_off);
  nop(gap_size);
  setSize(body_off - prologue_size);
  L("F" + std::to_string(funcidx_));

  // generate epilogue
  setSize(end_off);
  add(rsp, rsp_sub_);
  for (int32_t i = 4; i >= 0; i--) {
    if (used_callee_[i] != 0) {
      pop(to_xbyak_r64(reg::rbx + i));
    }
  }
  pop(rbp);
  ret();
}

int32_t wasm_jit_t::alloc_r(wasm_jit_val_t usage, uint8_t type) {
  int32_t start, end;
  if (type != WASM_TYPE_F64) {
    start = reg::rax;
    end = reg::xmm0;
  } else {
    start = reg::xmm0;
    end = reg::num_r;
  }
  uint32_t min_age = UINT32_MAX;
  int32_t min_r = start;
  for (int32_t r = start; r < end; r++) {
    if (ctx_.reg_[r].type == wasm_jit_type_t::INVALID) {
      acquire_r(r, usage);
      return r;
    }
    if (ctx_.reg_age_[r] < min_age) {
      min_age = ctx_.reg_age_[r];
      min_r = r;
    }
  }
  acquire_r(min_r, usage);
  return min_r;
}

void wasm_jit_t::acquire_r(int32_t index, wasm_jit_val_t usage) {
  wasm_jit_val_t old_usage = ctx_.reg_[index];
  int32_t s;
  switch (old_usage.type) {
    case wasm_jit_type_t::CALLER_DATA:
      used_callee_[index - reg::rbx] = 1;
      break;
    case wasm_jit_type_t::LOCAL_VAL:
      s = old_usage.index + 6;
      rbp_save_r(s, index);
      ctx_.local_loc_[old_usage.index] = {reg::on_stack, s};
      break;
    case wasm_jit_type_t::STACK_VAL:
      s = alloc_stk();
      rbp_save_r(s, index);
      ctx_.stack_loc_[old_usage.index] = {reg::on_stack, s};
      break;
    default:
      break;
  }
  ctx_.reg_[index] = usage;
  ctx_.reg_age_[index] = ctx_.age_++;
}

void wasm_jit_t::use_r(int32_t index, wasm_jit_val_t usage) {
  ctx_.reg_[index] = usage;
  ctx_.reg_age_[index] = ctx_.age_++;
}

void wasm_jit_t::release_r(int32_t index) {
  ctx_.reg_[index] = JIT_V_INVALID;
}

int32_t wasm_jit_t::alloc_stk() {
  int stack_size = ctx_.stack_valid_.size();
  for (int i = (int)(num_locals_ + 6); i < stack_size; i++) {
    if (ctx_.stack_valid_[i] == false) {
      ctx_.stack_valid_[i] = true;
      return i;
    }
  }
  // grow stack
  ctx_.stack_valid_.push_back(false);
  // new stack_size is stack_size +1
  // and we need sub rsp, 8*(stack_size-1) (rbp is saved)
  uint32_t new_rsp_sub = 8 * ((stack_size + 1) - 1);
  if (new_rsp_sub % 16 == 8) {
    new_rsp_sub += 8;
  }
  rsp_sub_ = std::max(rsp_sub_, new_rsp_sub);
  return stack_size;
}

void wasm_jit_t::release_stk(int32_t index) {
  ctx_.stack_valid_[index] = false;
}

void wasm_jit_t::acquire_stk(int32_t index) {
  ctx_.stack_valid_[index] = true;
}

wasm_jit_loc_t wasm_jit_t::peek_wasm_stk(uint32_t depth) {
  return ctx_.stack_loc_[ctx_.stack_loc_.size() - depth - 1];
}

wasm_jit_loc_t wasm_jit_t::pop_wasm_stk() {
  wasm_jit_loc_t result = ctx_.stack_loc_.back();
  ctx_.stack_loc_.pop_back();
  return result;
}

void wasm_jit_t::rbp_load_r(int32_t index, int32_t r, uint8_t type) {
  if (type != WASM_TYPE_F64) {
    if (type != WASM_TYPE_I32) {
      mov(to_xbyak_r64(r), qword[rbp - (index * 8)]);
    } else {
      mov(to_xbyak_r32(r), dword[rbp - (index * 8)]);
    }
  } else {
    movsd(to_xbyak_xmm(r), qword[rbp - (index * 8)]);
  }
  release_stk(index);
}

void wasm_jit_t::rbp_save_r(int32_t index, int32_t r) {
  if (r < reg::xmm0) {
    mov(qword[rbp - (index * 8)], to_xbyak_r64(r));
  } else {
    movsd(qword[rbp - (index * 8)], to_xbyak_xmm(r));
  }
  acquire_stk(index);
}

int32_t wasm_jit_t::pop_r(uint8_t type) {
  wasm_jit_loc_t loc = pop_wasm_stk();
  if (loc.type == reg::in_reg) {
    use_r(loc.index, JIT_V_INTER);
    return loc.index;
  }
  int32_t r = alloc_r(JIT_V_INTER, type);
  rbp_load_r(loc.index, r, type);
  return r;
}

void wasm_jit_t::pop_r(int32_t index, uint8_t type) {
  wasm_jit_loc_t loc = peek_wasm_stk(0);
  if (loc.type == reg::in_reg && loc.index == index) {
    pop_wasm_stk();
    use_r(loc.index, JIT_V_INTER);
    return;
  }
  acquire_r(index, JIT_V_INTER);
  // target may be moved
  loc = pop_wasm_stk();
  if (loc.type == reg::in_reg) {
    if (type != WASM_TYPE_F64) {
      if (type != WASM_TYPE_I32) {
        mov(to_xbyak_r64(index), to_xbyak_r64(loc.index));
      } else {
        mov(to_xbyak_r32(index), to_xbyak_r32(loc.index));
      }
    } else {
      movsd(to_xbyak_xmm(index), to_xbyak_xmm(loc.index));
    }
    release_r(loc.index);
    return;
  }
  rbp_load_r(loc.index, index, type);
}

int32_t wasm_jit_t::pop_binop_r32_r32(oo_op_t binop) {
  wasm_jit_loc_t a_loc = peek_wasm_stk(1);
  int32_t a;
  if (a_loc.type == reg::on_stack) {
    // move a to register and check b's location
    a = alloc_r(JIT_V_INTER, WASM_TYPE_I32);
    rbp_load_r(a_loc.index, a, WASM_TYPE_I32);
  } else {
    // no register allocation needed
    a = a_loc.index;
    use_r(a, JIT_V_INTER);
  }
  wasm_jit_loc_t b_loc = pop_wasm_stk();
  pop_wasm_stk();
  if (b_loc.type == reg::in_reg) {
    // ra = op(ra, rb)
    (this->*binop)(to_xbyak_r32(a), to_xbyak_r32(b_loc.index));
    release_r(b_loc.index);
  } else {
    // ra = op(ra, mb)
    (this->*binop)(to_xbyak_r32(a), dword[rbp - (b_loc.index * 8)]);
    release_stk(b_loc.index);
  }
  return a;
}

int32_t wasm_jit_t::pop_binop_xmm_xmm(xo_op_t binop) {
  wasm_jit_loc_t a_loc = peek_wasm_stk(1);
  int32_t a;
  if (a_loc.type == reg::on_stack) {
    // move a to register and check b's location
    a = alloc_r(JIT_V_INTER, WASM_TYPE_F64);
    rbp_load_r(a_loc.index, a, WASM_TYPE_F64);
  } else {
    // no register allocation needed
    a = a_loc.index;
    use_r(a, JIT_V_INTER);
  }
  wasm_jit_loc_t b_loc = pop_wasm_stk();
  pop_wasm_stk();
  if (b_loc.type == reg::in_reg) {
    // ra = op(ra, rb)
    (this->*binop)(to_xbyak_xmm(a), to_xbyak_xmm(b_loc.index));
    release_r(b_loc.index);
  } else {
    // ra = op(ra, mb)
    (this->*binop)(to_xbyak_xmm(a), dword[rbp - (b_loc.index * 8)]);
    release_stk(b_loc.index);
  }
  return a;
}

void wasm_jit_t::push_r(int32_t index) {
  ctx_.stack_loc_.push_back({reg::in_reg, index});
  ctx_.reg_[index] = JIT_V_STACK(ctx_.stack_loc_.size() - 1);
}

int32_t wasm_jit_t::alloc_special(wasm_jit_type_t t, size_t off) {
  for (int32_t i = 0; i < reg::num_r; i++) {
    if (ctx_.reg_[i].type == t) {
      return i;
    }
  }
  int32_t r = alloc_r({t, 0}, WASM_TYPE_I64);
  emit_mov_r_rip(r, off);
  return r;
}

int32_t wasm_jit_t::alloc_global_base() {
  return alloc_special(wasm_jit_type_t::GLOBAL_BASE, global_off);
}

int32_t wasm_jit_t::alloc_memory_base() {
  return alloc_special(wasm_jit_type_t::MEMORY_BASE, memory_off);
}

int32_t wasm_jit_t::alloc_memory_size() {
  return alloc_special(wasm_jit_type_t::MEMORY_SIZE, memory_size_off);
}

void wasm_jit_t::emit_lea_r_rip(int32_t r, size_t dest_offset) {
  size_t pos = getSize();
  lea(to_xbyak_r64(r), ptr[rip - 0x12345678]);
  size_t src_offset = getSize();
  setSize(pos);
  lea(to_xbyak_r64(r), ptr[rip - (int32_t)(src_offset - dest_offset)]);
  if (getSize() != src_offset) {
    throw std::runtime_error("Generated wrong rip addressing LEA");
  }
}

void wasm_jit_t::emit_mov_r_rip(int32_t r, size_t dest_offset) {
  size_t pos = getSize();
  mov(to_xbyak_r64(r), ptr[rip - 0x12345678]);
  size_t src_offset = getSize();
  setSize(pos);
  mov(to_xbyak_r64(r), ptr[rip - (int32_t)(src_offset - dest_offset)]);
  if (getSize() != src_offset) {
    throw std::runtime_error("Generated wrong rip addressing MOV");
  }
}

void wasm_jit_t::emit_unreachable() {
  ud2();
}

void wasm_jit_t::emit_nop() {}

void wasm_jit_t::emit_drop() {
  wasm_jit_loc_t loc = pop_wasm_stk();
  if (loc.type == reg::in_reg) {
    release_r(loc.index);
  } else {
    release_stk(loc.index);
  }
}

void wasm_jit_t::emit_select() {
  int32_t c = pop_r(WASM_TYPE_I32);
  int32_t b = pop_r(WASM_TYPE_I32);
  int32_t a = pop_r(WASM_TYPE_I32);
  test(to_xbyak_r32(c), to_xbyak_r32(c));
  mov(to_xbyak_r32(c), to_xbyak_r32(b));
  cmovne(to_xbyak_r32(c), to_xbyak_r32(a));
  release_r(a);
  release_r(b);
  push_r(c);
}

void wasm_jit_t::emit_local_get() {
  uint32_t idx = read_u32leb(&buf_);
  uint8_t type = local_types_[idx];
  wasm_jit_loc_t* local_loc = &ctx_.local_loc_[idx];
  if (local_loc->type == reg::in_reg) {
    // r -> (m, r)
    rbp_save_r(idx + 6, local_loc->index);
    push_r(local_loc->index);
    *local_loc = {reg::on_stack, (int32_t)idx + 6};
    return;
  }
  // m -> (m, r)
  int32_t r = alloc_r(JIT_V_INTER, type);
  rbp_load_r(local_loc->index, r, type);
  push_r(r);
}

void wasm_jit_t::emit_local_set() {
  uint32_t idx = read_u32leb(&buf_);
  uint8_t type = local_types_[idx];
  wasm_jit_loc_t* local_loc = &ctx_.local_loc_[idx];
  wasm_jit_loc_t stack_loc = pop_wasm_stk();
  if (local_loc->type == reg::in_reg) {
    if (stack_loc.type == reg::in_reg) {
      // r -> r
      use_r(stack_loc.index, JIT_V_LOCAL(idx));
      release_r(local_loc->index);
      local_loc->index = stack_loc.index;
    } else {
      // m -> r
      rbp_load_r(stack_loc.index, local_loc->index, type);
    }
    return;
  }
  if (stack_loc.type == reg::in_reg) {
    // r -> m
    rbp_save_r(local_loc->index, stack_loc.index);
    release_r(stack_loc.index);
  } else {
    // m -> m, move local to r
    int32_t r = alloc_r(JIT_V_LOCAL(idx), type);
    rbp_load_r(stack_loc.index, r, type);
    *local_loc = {reg::on_stack, r};
  }
}

void wasm_jit_t::emit_local_tee() {
  uint32_t idx = read_u32leb(&buf_);
  uint8_t type = local_types_[idx];
  wasm_jit_loc_t* local_loc = &ctx_.local_loc_[idx];
  wasm_jit_loc_t stack_loc = peek_wasm_stk(0);
  if (local_loc->type == reg::in_reg) {
    if (stack_loc.type == reg::in_reg) {
      // r -> r
      if (type != WASM_TYPE_F64) {
        mov(to_xbyak_r64(local_loc->index), to_xbyak_r64(stack_loc.index));
      } else {
        mov(to_xbyak_xmm(local_loc->index), to_xbyak_xmm(stack_loc.index));
      }
    } else {
      // m -> r, need to remark stack as valid
      rbp_load_r(stack_loc.index, local_loc->index, type);
      acquire_stk(stack_loc.index);
    }
    return;
  }
  if (stack_loc.type == reg::in_reg) {
    // r -> m
    rbp_save_r(local_loc->index, stack_loc.index);
  } else {
    // m -> m, move local to r, and need to remark stack as valid
    int32_t r = alloc_r(JIT_V_LOCAL(idx), type);
    rbp_load_r(stack_loc.index, r, type);
    acquire_stk(stack_loc.index);
    *local_loc = {reg::on_stack, r};
  }
}

// globals are always in memory
void wasm_jit_t::emit_global_get() {
  uint32_t idx = read_u32leb(&buf_);
  uint8_t type = mod_->globals_[idx].type_.type_;
  int32_t r = alloc_global_base();
  if (type != WASM_TYPE_F64) {
    if (type != WASM_TYPE_I32) {
      mov(to_xbyak_r64(r), qword[to_xbyak_r64(r) + (idx * 8)]);
      push_r(r);
    } else {
      mov(to_xbyak_r32(r), dword[to_xbyak_r64(r) + (idx * 8)]);
      push_r(r);
    }
  } else {
    int32_t s = alloc_r(JIT_V_INTER, WASM_TYPE_F64);
    movsd(to_xbyak_xmm(s), qword[to_xbyak_r64(r) + (idx * 8)]);
    push_r(s);
    release_r(r);
  }
}

void wasm_jit_t::emit_global_set() {
  uint32_t idx = read_u32leb(&buf_);
  uint8_t type = mod_->globals_[idx].type_.type_;
  int32_t r = alloc_global_base();
  // load global base must happen before here or stack loc will change
  wasm_jit_loc_t stack_loc = pop_wasm_stk();
  int32_t s;
  if (stack_loc.type == reg::in_reg) {
    // r -> m
    s = stack_loc.index;
  } else {
    // m -> m
    s = alloc_r(JIT_V_INTER, type);
    rbp_load_r(stack_loc.index, s, type);
  }
  if (type != WASM_TYPE_F64) {
    if (type != WASM_TYPE_I32) {
      mov(qword[to_xbyak_r64(r) + (idx * 8)], to_xbyak_r64(s));
    } else {
      mov(dword[to_xbyak_r64(r) + (idx * 8)], to_xbyak_r32(s));
    }
  } else {
    movsd(qword[to_xbyak_r64(r) + (idx * 8)], to_xbyak_xmm(s));
  }
  release_r(s);
}

void wasm_jit_t::emit_i32_load() {
  read_u32leb(&buf_);
  uint32_t addend = read_u32leb(&buf_);
  uint32_t v_size = 4;
  switch (opcode_) {
    case WASM_OP_I32_LOAD16_S:
    case WASM_OP_I32_LOAD16_U:
      v_size = 2;
      break;
    case WASM_OP_I32_LOAD8_S:
    case WASM_OP_I32_LOAD8_U:
      v_size = 1;
      break;
    default:
      break;
  }
  int32_t offset = pop_r(WASM_TYPE_I32);
  int32_t m = alloc_memory_base();
  Xbyak::RegExp e;
  if (enable_check_ == true) {
    int32_t mem_size = alloc_memory_size();
    inLocalLabel();
    cmp(to_xbyak_r32(mem_size), addend);
    jae(".L0");
    call("trap");
    L(".L0");
    cmp(to_xbyak_r32(mem_size), to_xbyak_r32(offset));
    jae(".L1");
    call("trap");
    L(".L1");
    add(to_xbyak_r32(offset), addend);
    push(to_xbyak_r64(offset));
    add(to_xbyak_r32(offset), v_size);
    cmp(to_xbyak_r32(mem_size), to_xbyak_r32(offset));
    jae(".L2");
    call("trap");
    L(".L2");
    outLocalLabel();
    pop(to_xbyak_r64(offset));
    e = to_xbyak_r64(m) + to_xbyak_r64(offset);
  } else {
    mov(to_xbyak_r32(offset), to_xbyak_r32(offset));
    e = to_xbyak_r64(m) + to_xbyak_r64(offset) + (int32_t)addend;
  }
  switch (opcode_) {
    case WASM_OP_I32_LOAD16_S:
      movsx(to_xbyak_r32(offset), word[e]);
      break;
    case WASM_OP_I32_LOAD16_U:
      mov(to_xbyak_r16(offset), word[e]);
      and_(to_xbyak_r32(offset), 0xFFFF);
      break;
    case WASM_OP_I32_LOAD8_S:
      movsx(to_xbyak_r32(offset), byte[e]);
      break;
    case WASM_OP_I32_LOAD8_U:
      mov(to_xbyak_r8(offset), byte[e]);
      and_(to_xbyak_r32(offset), 0xFF);
      break;
    default:
      mov(to_xbyak_r32(offset), dword[e]);
      break;
  }
  push_r(offset);
}

void wasm_jit_t::emit_f64_load() {
  read_u32leb(&buf_);
  uint32_t addend = read_u32leb(&buf_);
  int32_t xmm = alloc_r(JIT_V_INTER, WASM_TYPE_F64);
  int32_t offset = pop_r(WASM_TYPE_I32);
  int32_t m = alloc_memory_base();
  Xbyak::RegExp e;
  if (enable_check_ == true) {
    int32_t mem_size = alloc_memory_size();
    inLocalLabel();
    cmp(to_xbyak_r32(mem_size), addend);
    jae(".L0");
    call("trap");
    L(".L0");
    cmp(to_xbyak_r32(mem_size), to_xbyak_r32(offset));
    jae(".L1");
    call("trap");
    L(".L1");
    add(to_xbyak_r32(offset), addend);
    push(to_xbyak_r64(offset));
    add(to_xbyak_r32(offset), sizeof(double));
    cmp(to_xbyak_r32(mem_size), to_xbyak_r32(offset));
    jae(".L2");
    call("trap");
    L(".L2");
    outLocalLabel();
    pop(to_xbyak_r64(offset));
    e = to_xbyak_r64(m) + to_xbyak_r64(offset);
  } else {
    mov(to_xbyak_r32(offset), to_xbyak_r32(offset));
    e = to_xbyak_r64(m) + to_xbyak_r64(offset) + (int32_t)addend;
  }
  movsd(to_xbyak_xmm(xmm), qword[e]);
  push_r(xmm);
  release_r(offset);
}

void wasm_jit_t::emit_i64_store() {
  read_u32leb(&buf_);
  uint32_t addend = read_u32leb(&buf_);
  uint32_t v_size = 4;
  uint8_t v_type = WASM_TYPE_I32;
  switch (opcode_) {
    case WASM_OP_I64_STORE:
      v_type = WASM_TYPE_I64;
      v_size = 8;
      break;
    case WASM_OP_I64_STORE32:
      v_type = WASM_TYPE_I64;
      v_size = 4;
      break;
    case WASM_OP_I64_STORE16:
      v_type = WASM_TYPE_I64;
      v_size = 2;
      break;
    case WASM_OP_I64_STORE8:
      v_type = WASM_TYPE_I64;
      v_size = 1;
      break;
    case WASM_OP_I32_STORE:
      v_type = WASM_TYPE_I32;
      v_size = 4;
      break;
    case WASM_OP_I32_STORE16:
      v_type = WASM_TYPE_I32;
      v_size = 2;
      break;
    case WASM_OP_I32_STORE8:
      v_type = WASM_TYPE_I32;
      v_size = 1;
      break;
    default:
      break;
  }
  int32_t v = pop_r(v_type);
  int32_t offset = pop_r(WASM_TYPE_I32);
  int32_t m = alloc_memory_base();
  Xbyak::RegExp e;
  int32_t mem_size;
  if (enable_check_ == true) {
    mem_size = alloc_memory_size();
    inLocalLabel();
    cmp(to_xbyak_r32(mem_size), addend);
    jae(".L0");
    call("trap");
    L(".L0");
    cmp(to_xbyak_r32(mem_size), to_xbyak_r32(offset));
    jae(".L1");
    call("trap");
    L(".L1");
    add(to_xbyak_r32(offset), addend);
    push(to_xbyak_r64(offset));
    add(to_xbyak_r32(offset), v_size);
    cmp(to_xbyak_r32(mem_size), to_xbyak_r32(offset));
    jae(".L2");
    call("trap");
    L(".L2");
    outLocalLabel();
    pop(to_xbyak_r64(offset));
    e = to_xbyak_r64(m) + to_xbyak_r64(offset);
  } else {
    mov(to_xbyak_r32(offset), to_xbyak_r32(offset));
    e = to_xbyak_r64(m) + to_xbyak_r64(offset) + (int32_t)addend;
  }
  switch (opcode_) {
    case WASM_OP_I64_STORE:
      mov(qword[e], to_xbyak_r64(v));
      break;
    case WASM_OP_I64_STORE32:
    case WASM_OP_I32_STORE:
      mov(dword[e], to_xbyak_r32(v));
      break;
    case WASM_OP_I64_STORE16:
    case WASM_OP_I32_STORE16:
      mov(word[e], to_xbyak_r16(v));
      break;
    case WASM_OP_I64_STORE8:
    case WASM_OP_I32_STORE8:
      mov(byte[e], to_xbyak_r8(v));
      break;
    default:
      break;
  }
  release_r(offset);
  release_r(v);
}

void wasm_jit_t::emit_f64_store() {
  read_u32leb(&buf_);
  uint32_t addend = read_u32leb(&buf_);
  int32_t v = pop_r(WASM_TYPE_F64);
  int32_t offset = pop_r(WASM_TYPE_I32);
  int32_t m = alloc_memory_base();
  Xbyak::RegExp e;
  int32_t mem_size;
  if (enable_check_ == true) {
    mem_size = alloc_memory_size();
    inLocalLabel();
    cmp(to_xbyak_r32(mem_size), addend);
    jae(".L0");
    call("trap");
    L(".L0");
    cmp(to_xbyak_r32(mem_size), to_xbyak_r32(offset));
    jae(".L1");
    call("trap");
    L(".L1");
    add(to_xbyak_r32(offset), addend);
    push(to_xbyak_r64(offset));
    add(to_xbyak_r32(offset), sizeof(double));
    cmp(to_xbyak_r32(mem_size), to_xbyak_r32(offset));
    jae(".L2");
    call("trap");
    L(".L2");
    outLocalLabel();
    pop(to_xbyak_r64(offset));
    movsxd(to_xbyak_r64(offset), to_xbyak_r32(offset));
    e = to_xbyak_r64(m) + to_xbyak_r64(offset);
  } else {
    mov(to_xbyak_r32(offset), to_xbyak_r32(offset));
    e = to_xbyak_r64(m) + to_xbyak_r64(offset) + (int32_t)addend;
  }
  movsd(qword[e], to_xbyak_xmm(v));
  release_r(offset);
  release_r(v);
}

void wasm_jit_t::emit_i32_numeric() {
  static o_op_t setcc[] = {&wasm_jit_t::sete,  &wasm_jit_t::setne,
                           &wasm_jit_t::setl,  &wasm_jit_t::setb,
                           &wasm_jit_t::setg,  &wasm_jit_t::seta,
                           &wasm_jit_t::setle, &wasm_jit_t::setbe,
                           &wasm_jit_t::setge, &wasm_jit_t::setae};
  static oo_op_t numeric[] = {
      &wasm_jit_t::add, &wasm_jit_t::sub, nullptr, nullptr,
      nullptr,          nullptr,          nullptr, &wasm_jit_t::and_,
      &wasm_jit_t::or_, &wasm_jit_t::xor_};
  int32_t b;
  int32_t a;
  switch (opcode_) {
    case WASM_OP_I32_EQZ:
      b = pop_r(WASM_TYPE_I32);
      test(to_xbyak_r32(b), to_xbyak_r32(b));
      sete(to_xbyak_r8(b));
      movsx(to_xbyak_r32(b), to_xbyak_r8(b));
      push_r(b);
      break;
    case WASM_OP_I32_EQ:
    case WASM_OP_I32_NE:
    case WASM_OP_I32_LT_S:
    case WASM_OP_I32_LT_U:
    case WASM_OP_I32_GT_S:
    case WASM_OP_I32_GT_U:
    case WASM_OP_I32_LE_S:
    case WASM_OP_I32_LE_U:
    case WASM_OP_I32_GE_S:
    case WASM_OP_I32_GE_U:
      a = pop_binop_r32_r32(static_cast<oo_op_t>(&wasm_jit_t::cmp));
      (this->*setcc[opcode_ - WASM_OP_I32_EQ])(to_xbyak_r8(a));
      movsx(to_xbyak_r32(a), to_xbyak_r8(a));
      push_r(a);
      break;
    case WASM_OP_I32_CLZ:
      // https://stackoverflow.com/questions/9353973/implementation-of-builtin-clz
      inLocalLabel();
      b = pop_r(WASM_TYPE_I32);
      test(to_xbyak_r32(b), to_xbyak_r32(b));
      je(".clz_zero");
      bsr(to_xbyak_r32(b), to_xbyak_r32(b));
      xor_(to_xbyak_r32(b), 31);
      jmp(".clz_done");
      L(".clz_zero");
      mov(to_xbyak_r32(b), 32);
      L(".clz_done");
      push_r(b);
      outLocalLabel();
      break;
    case WASM_OP_I32_CTZ:
      inLocalLabel();
      b = pop_r(WASM_TYPE_I32);
      test(to_xbyak_r32(b), to_xbyak_r32(b));
      je(".ctz_zero");
      bsf(to_xbyak_r32(b), to_xbyak_r32(b));
      jmp(".ctz_done");
      L(".ctz_zero");
      mov(to_xbyak_r32(b), 32);
      L(".ctz_done");
      push_r(b);
      outLocalLabel();
      break;
    case WASM_OP_I32_POPCNT:
      b = pop_r(WASM_TYPE_I32);
      popcnt(to_xbyak_r32(b), to_xbyak_r32(b));
      push_r(b);
      break;
    case WASM_OP_I32_ADD:
    case WASM_OP_I32_SUB:
    case WASM_OP_I32_AND:
    case WASM_OP_I32_OR:
    case WASM_OP_I32_XOR:
      a = pop_binop_r32_r32(numeric[opcode_ - WASM_OP_I32_ADD]);
      push_r(a);
      break;
    case WASM_OP_I32_MUL:
      pop_r(reg::rdx, WASM_TYPE_I32);
      pop_r(reg::rax, WASM_TYPE_I32);
      imul(edx);
      release_r(reg::rdx);
      push_r(reg::rax);
      break;
    case WASM_OP_I32_DIV_S:
      acquire_r(reg::rdx, JIT_V_INTER);
      b = pop_r(WASM_TYPE_I32);
      pop_r(reg::rax, WASM_TYPE_I32);
      if (enable_check_ == true) {
        // see i32_div_s() at weerun_inst.cpp:372
        inLocalLabel();
        test(to_xbyak_r32(b), to_xbyak_r32(b));
        jne(".L0");
        call("trap");
        L(".L0");
        cmp(eax, 0x80000000);
        jne(".L1");
        cmp(to_xbyak_r32(b), 0xffffffff);
        jne(".L1");
        call("trap");
        L(".L1");
        outLocalLabel();
      }
      cdq();
      idiv(to_xbyak_r32(b));
      release_r(b);
      release_r(reg::rdx);
      push_r(reg::rax);
      break;
    case WASM_OP_I32_DIV_U:
      acquire_r(reg::rdx, JIT_V_INTER);
      b = pop_r(WASM_TYPE_I32);
      pop_r(reg::rax, WASM_TYPE_I32);
      if (enable_check_ == true) {
        // see i32_div_s() at weerun_inst.cpp:372
        inLocalLabel();
        test(to_xbyak_r32(b), to_xbyak_r32(b));
        jne(".L0");
        call("trap");
        L(".L0");
        outLocalLabel();
      }
      xor_(rdx, rdx);
      div(to_xbyak_r32(b));
      release_r(b);
      release_r(reg::rdx);
      push_r(reg::rax);
      break;
    case WASM_OP_I32_REM_S:
      inLocalLabel();
      acquire_r(reg::rdx, JIT_V_INTER);
      b = pop_r(WASM_TYPE_I32);
      pop_r(reg::rax, WASM_TYPE_I32);
      if (enable_check_ == true) {
        // see i32_div_s() at weerun_inst.cpp:372
        test(to_xbyak_r32(b), to_xbyak_r32(b));
        jne(".L0");
        call("trap");
        L(".L0");
        cmp(eax, 0x80000000);
        jne(".L1");
        cmp(to_xbyak_r32(b), 0xffffffff);
        jne(".L1");
        xor_(rdx, rdx);
        jmp(".L2");
      }
      L(".L1");
      cdq();
      idiv(to_xbyak_r32(b));
      L(".L2");
      release_r(b);
      release_r(reg::rax);
      push_r(reg::rdx);
      outLocalLabel();
      break;
    case WASM_OP_I32_REM_U:
      acquire_r(reg::rdx, JIT_V_INTER);
      b = pop_r(WASM_TYPE_I32);
      pop_r(reg::rax, WASM_TYPE_I32);
      if (enable_check_ == true) {
        // see i32_div_s() at weerun_inst.cpp:372
        inLocalLabel();
        test(to_xbyak_r64(b), to_xbyak_r64(b));
        jne(".L0");
        call("trap");
        L(".L0");
        outLocalLabel();
      }
      xor_(rdx, rdx);
      div(to_xbyak_r32(b));
      release_r(b);
      release_r(reg::rax);
      push_r(reg::rdx);
      break;
    case WASM_OP_I32_EXTEND16_S:
      b = pop_r(WASM_TYPE_I32);
      movsx(to_xbyak_r32(b), to_xbyak_r16(b));
      push_r(b);
      break;
    case WASM_OP_I32_EXTEND8_S:
      b = pop_r(WASM_TYPE_I32);
      movsx(to_xbyak_r32(b), to_xbyak_r8(b));
      push_r(b);
      break;
    default:
      throw std::runtime_error("Opcode not implemented");
  }
}

void wasm_jit_t::emit_i32_shift() {
  static or8_op_t shift[] = {&wasm_jit_t::shl, &wasm_jit_t::sar,
                             &wasm_jit_t::shr, &wasm_jit_t::rol,
                             &wasm_jit_t::ror};
  pop_r(reg::rcx, WASM_TYPE_I32);
  and_(ecx, 0x1F);
  int32_t a = pop_r(WASM_TYPE_I32);
  (this->*shift[opcode_ - WASM_OP_I32_SHL])(to_xbyak_r32(a), cl);
  push_r(a);
  release_r(reg::rcx);
}

void wasm_jit_t::emit_i32_const() {
  int32_t x = read_i32leb(&buf_);
  int32_t r = alloc_r(JIT_V_INTER, WASM_TYPE_I32);
  mov(to_xbyak_r32(r), x);
  push_r(r);
}

void wasm_jit_t::emit_i64_const() {
  int64_t x = read_i64leb(&buf_);
  int32_t r = alloc_r(JIT_V_INTER, WASM_TYPE_I64);
  mov(to_xbyak_r64(r), x);
  push_r(r);
}

void wasm_jit_t::emit_call_internal(wasm_func_decl_t* sig,
                                    int is_direct,
                                    int index) {
  int num_r64 = 0;
  int num_f64 = 0;
  int num_stack_r64 = 0;
  int num_stack_f64 = 0;
  int num_args = sig->args_.size();
  int32_t arg_size = 0;
  for (uint8_t arg : sig->args_) {
    if (arg != WASM_TYPE_F64) {
      if (num_r64 < 6) {
        num_r64++;
      } else {
        num_stack_r64++;
      }
    } else {
      if (num_f64 < 8) {
        num_f64++;
      } else {
        num_stack_f64++;
      }
    }
  }
  for (int32_t r = reg::rax; r < reg::rbx; r++) {
    acquire_r(r, JIT_V_INVALID);
  }
  for (int32_t r = reg::xmm0; r < reg::num_r; r++) {
    acquire_r(r, JIT_V_INVALID);
  }
  if (is_direct != 1) {
    pop_r(reg::r10, WASM_TYPE_I32);
    if (enable_check_ == true) {
      // check function type
      inLocalLabel();
      emit_mov_r_rip(reg::rax, table_size_off);
      cmp(r10, rax);
      jb(".L0");
      call("trap");
      L(".L0");
      shl(r10, 1);
      emit_mov_r_rip(reg::rax, table_off);
      mov(r11, qword[rax + r10 * 8]);
      cmp(r11, index);
      je(".L1");
      call("trap");
      L(".L1");
      outLocalLabel();
    } else {
      shl(r10, 1);
    }
  }
  arg_size = num_stack_f64 + num_stack_r64;
  if (arg_size % 2 == 1) {
    sub(rsp, 0x8);
    arg_size++;
  }
  for (int i = num_args - 1; i >= 0; i--) {
    if (sig->args_[i] != WASM_TYPE_F64) {
      if (num_stack_r64 > 0) {
        wasm_jit_loc_t loc = pop_wasm_stk();
        if (loc.type == reg::in_reg) {
          push(to_xbyak_r64(loc.index));
          release_r(loc.index);
        } else {
          rbp_load_r(loc.index, reg::rax, WASM_TYPE_I64);
          push(to_xbyak_r64(reg::rax));
        }
        num_stack_r64--;
      } else {
        num_r64--;
        pop_r(sysv_args[num_r64], WASM_TYPE_I64);
      }
    } else {
      if (num_stack_f64 > 0) {
        wasm_jit_loc_t loc = pop_wasm_stk();
        if (loc.type == reg::in_reg) {
          sub(rsp, 0x8);
          movsd(qword[rsp], to_xbyak_xmm(loc.index));
          release_r(loc.index);
        } else {
          rbp_load_r(loc.index, reg::rax, WASM_TYPE_I64);
          push(to_xbyak_r64(reg::rax));
        }
        num_stack_f64--;
      } else {
        num_f64--;
        pop_r(reg::xmm0 + num_f64, WASM_TYPE_F64);
      }
    }
  }
  if (is_direct == 1) {
    std::string f_label = "F" + std::to_string(index);
    call(f_label);
  } else {
    emit_mov_r_rip(reg::rax, table_off);
    call(qword[rax + r10 * 8 + 8]);
  }
  if (arg_size != 0) {
    add(rsp, arg_size * 8);
  }
  if (sig->ret_.empty() == false) {
    if (sig->ret_[0] != WASM_TYPE_F64) {
      push_r(reg::rax);
    } else {
      push_r(reg::xmm0);
    }
  }
}

void wasm_jit_t::emit_call() {
  uint32_t funcidx = read_u32leb(&buf_);
  wasm_func_t* f = &mod_->funcs_[funcidx];
  emit_call_internal(f->sig_, 1, funcidx);
}

void wasm_jit_t::emit_call_indirect() {
  read_u32leb(&buf_);
  uint32_t typeidx = read_u32leb(&buf_);
  emit_call_internal(&mod_->sigs_[typeidx], 0, typeidx);
}

void wasm_jit_t::emit_pop_result() {
  if (f_->sig_->ret_.empty() == false) {
    if (f_->sig_->ret_[0] != WASM_TYPE_F64) {
      pop_r(reg::rax, f_->sig_->ret_[0]);
    } else {
      pop_r(reg::xmm0, f_->sig_->ret_[0]);
    }
  }
}

void wasm_jit_t::emit_return() {
  emit_pop_result();
  jmp("F" + std::to_string(funcidx_) + "E", T_NEAR);
}

void wasm_jit_t::emit_block() {
  read_u8(&buf_);
  if (opcode_ == WASM_OP_LOOP) {
    block_stack_.push_back({1, next_bi_, ctx_});
    L("F" + std::to_string(funcidx_) + "L" + std::to_string(next_bi_++));
  } else {
    block_stack_.push_back({0, next_bi_});
    next_bi_++;
  }
}

void wasm_jit_t::emit_end() {
  block_t* b = &block_stack_.back();
  wasm_jit_ctx_t saved_ctx = ctx_;
  std::string label("F" + std::to_string(funcidx_) + "L" +
                    std::to_string(b->index));
  std::string end_label(label + "E");
  int num_brs = b->br_ctxs.size();
  if (num_brs > 0) {
    jmp(end_label, T_NEAR);
    for (int i = 0; i < num_brs; i++) {
      L(label + "T" + std::to_string(i));
      if (b->loop == 0) {
        restore_state(&b->br_ctxs[i], &ctx_);
        // let last one fall through
        if (i < num_brs - 1) {
          jmp(end_label, T_NEAR);
        }
      } else {
        restore_state(&b->br_ctxs[i], &b->saved_ctx);
        jmp(label, T_NEAR);
      }
      ctx_ = saved_ctx;
    }
  }
  L(end_label);
  block_stack_.pop_back();
  if (block_stack_.empty() == true) {
    emit_pop_result();
    stop_ = 1;
    return;
  }
}

void wasm_jit_t::emit_br() {
  uint32_t depth = read_u32leb(&buf_);
  block_t* b = (&block_stack_.back()) - depth;
  uint32_t t = b->br_ctxs.size();
  jmp("F" + std::to_string(funcidx_) + "L" + std::to_string(b->index) + "T" +
          std::to_string(t),
      T_NEAR);
  b->br_ctxs.push_back(ctx_);
}

void wasm_jit_t::emit_br_if() {
  uint32_t depth = read_u32leb(&buf_);
  block_t* b = (&block_stack_.back()) - depth;
  uint32_t t = b->br_ctxs.size();
  int32_t r = pop_r(WASM_TYPE_I32);
  test(to_xbyak_r32(r), to_xbyak_r32(r));
  jne("F" + std::to_string(funcidx_) + "L" + std::to_string(b->index) + "T" +
          std::to_string(t),
      T_NEAR);
  b->br_ctxs.push_back(ctx_);
}

void wasm_jit_t::restore_state(wasm_jit_ctx_t* from, wasm_jit_ctx_t* to) {
  wasm_jit_ctx_t tmp_to = *to;  // to may be curent context
  ctx_ = *from;
  int num_wasm_local = ctx_.local_loc_.size();
  int num_wasm_stack = ctx_.stack_loc_.size();
  int num_wasm_stack_dst = tmp_to.stack_loc_.size();
  if (num_wasm_stack < num_wasm_stack_dst) {
    throw std::runtime_error("jmp dst have more stack variables");
  }
  acquire_r(reg::rax, JIT_V_INVALID);  // use rax for m -> m
  // save all stack variables
  if (num_wasm_stack_dst != 0) {
    sub(rsp, num_wasm_stack_dst * 0x8);
  }
  for (int i = 0; i < num_wasm_stack_dst; i++) {
    wasm_jit_loc_t loc = ctx_.stack_loc_[i];
    wasm_jit_loc_t to_loc = tmp_to.stack_loc_[i];
    if (loc == to_loc) {
      continue;
    }
    if (loc.type == reg::in_reg) {
      if (loc.index < reg::xmm0) {
        // r -> m
        mov(qword[rsp + (i * 0x8)], to_xbyak_r64(loc.index));
      } else {
        // xmm -> m
        movsd(qword[rsp + (i * 0x8)], to_xbyak_xmm(loc.index));
      }
    } else {
      // m -> m
      rbp_load_r(loc.index, reg::rax, WASM_TYPE_I64);
      mov(qword[rsp + (i * 0x8)], rax);
    }
  }
  // save all local variables, local variables have fixed stack pos
  for (int i = 0; i < num_wasm_local; i++) {
    wasm_jit_loc_t loc = ctx_.local_loc_[i];
    wasm_jit_loc_t to_loc = tmp_to.local_loc_[i];
    if (loc == to_loc) {
      continue;
    }
    if (loc.type == reg::in_reg) {
      rbp_save_r(i + 6, loc.index);
    }
  }
  // restore stack variables
  for (int i = 0; i < num_wasm_stack_dst; i++) {
    wasm_jit_loc_t loc = tmp_to.stack_loc_[i];
    wasm_jit_loc_t from_loc = ctx_.stack_loc_[i];
    if (loc == from_loc) {
      continue;
    }
    if (loc.type == reg::in_reg) {
      if (loc.index < reg::xmm0) {
        // r -> m
        mov(to_xbyak_r64(loc.index), qword[rsp + (i * 0x8)]);
      } else {
        // xmm -> m
        movsd(to_xbyak_xmm(loc.index), qword[rsp + (i * 0x8)]);
      }
    } else {
      // m -> m
      mov(rax, qword[rsp + (i * 0x8)]);
      rbp_save_r(loc.index, reg::rax);
    }
  }
  if (num_wasm_stack_dst != 0) {
    add(rsp, num_wasm_stack_dst * 0x8);
  }
  // restore local variables
  for (int i = 0; i < num_wasm_local; i++) {
    wasm_jit_loc_t loc = tmp_to.local_loc_[i];
    wasm_jit_loc_t from_loc = ctx_.local_loc_[i];
    if (loc == from_loc) {
      continue;
    }
    if (loc.type == reg::in_reg) {
      rbp_load_r(i + 6, loc.index, local_types_[i]);
    }
  }
  // restore special register values
  for (int i = 0; i < reg::num_r; i++) {
    switch (tmp_to.reg_[i].type) {
      case wasm_jit_type_t::GLOBAL_BASE:
        emit_mov_r_rip(i, global_off);
        break;
      case wasm_jit_type_t::MEMORY_BASE:
        emit_mov_r_rip(i, memory_off);
        break;
      case wasm_jit_type_t::MEMORY_SIZE:
        emit_mov_r_rip(i, memory_size_off);
        break;
      default:
        break;
    }
  }
}

// reimplement code generated by GDB, I don't know how it works...
void wasm_jit_t::emit_i32_convert() {
  inLocalLabel();
  int32_t x0i = pop_r(WASM_TYPE_F64);
  int32_t x1i = alloc_r(JIT_V_INTER, WASM_TYPE_F64);
  Xbyak::Xmm x0 = to_xbyak_xmm(x0i);
  Xbyak::Xmm x1 = to_xbyak_xmm(x1i);
  int32_t r = alloc_r(JIT_V_INTER, WASM_TYPE_I64);
  movapd(x1, x0);
  emit_lea_r_rip(r, f64_const_off);
  andpd(x1, xword[to_xbyak_r64(r)]);
  ucomisd(x1, qword[to_xbyak_r64(r) + 0x10]);
  ja(".trap");
  ucomisd(x1, x1);
  jp(".trap");
  movsd(x1, qword[to_xbyak_r64(r) +
                  (opcode_ == WASM_OP_I32_TRUNC_F64_S ? 0x20 : 0x30)]);
  comisd(x1, x0);
  jae(".trap");
  comisd(x0, qword[to_xbyak_r64(r) +
                   (opcode_ == WASM_OP_I32_TRUNC_F64_S ? 0x18 : 0x28)]);
  jae(".trap");
  if (opcode_ == WASM_OP_I32_TRUNC_F64_S) {
    cvttsd2si(to_xbyak_r32(r), x0);
  } else {
    cvttsd2si(to_xbyak_r64(r), x0);
  }
  jmp(".safe");
  L(".trap");
  call("trap");
  L(".safe");
  push_r(r);
  release_r(x0i);
  release_r(x1i);
  outLocalLabel();
}

void wasm_jit_t::emit_f64_const() {
  uint64_t x = *(uint64_t*)(buf_.ptr);
  buf_.ptr += sizeof(uint64_t);
  int32_t s = alloc_r(JIT_V_INTER, WASM_TYPE_I64);
  int32_t r = alloc_r(JIT_V_INTER, WASM_TYPE_F64);
  mov(to_xbyak_r64(s), x);
  push(to_xbyak_r64(s));
  movsd(to_xbyak_xmm(r), qword[rsp]);
  add(rsp, 0x8);
  release_r(s);
  push_r(r);
}

void wasm_jit_t::emit_f64_compute() {
  static o_op_t setcc[] = {&wasm_jit_t::sete,  &wasm_jit_t::setne,
                           &wasm_jit_t::setb,  &wasm_jit_t::seta,
                           &wasm_jit_t::setbe, &wasm_jit_t::setae};
  static xo_op_t fnumeric[] = {&wasm_jit_t::addsd, &wasm_jit_t::subsd,
                               &wasm_jit_t::mulsd, &wasm_jit_t::divsd};
  int32_t b;
  int32_t a;
  int32_t r;
  switch (opcode_) {
    case WASM_OP_F64_EQ:
    case WASM_OP_F64_NE:
    case WASM_OP_F64_LT:
    case WASM_OP_F64_GT:
    case WASM_OP_F64_LE:
    case WASM_OP_F64_GE:
      b = pop_r(WASM_TYPE_F64);
      a = pop_r(WASM_TYPE_F64);
      r = alloc_r(JIT_V_INTER, WASM_TYPE_I64);
      comisd(to_xbyak_xmm(a), to_xbyak_xmm(b));
      (this->*setcc[opcode_ - WASM_OP_F64_EQ])(to_xbyak_r8(r));
      movsx(to_xbyak_r32(r), to_xbyak_r8(r));
      release_r(b);
      release_r(a);
      push_r(r);
      break;
    case WASM_OP_F64_ADD:
    case WASM_OP_F64_SUB:
    case WASM_OP_F64_MUL:
    case WASM_OP_F64_DIV:
      a = pop_binop_xmm_xmm(fnumeric[opcode_ - WASM_OP_F64_ADD]);
      push_r(a);
      break;
    case WASM_OP_F64_CONVERT_I32_S:
      b = pop_r(WASM_TYPE_I32);
      r = alloc_r(JIT_V_INTER, WASM_TYPE_F64);
      cvtsi2sd(to_xbyak_xmm(r), to_xbyak_r32(b));
      push_r(r);
      release_r(b);
      break;
    case WASM_OP_F64_CONVERT_I32_U:
      b = pop_r(WASM_TYPE_I64);
      r = alloc_r(JIT_V_INTER, WASM_TYPE_F64);
      mov(to_xbyak_r32(b), to_xbyak_r32(b));
      cvtsi2sd(to_xbyak_xmm(r), to_xbyak_r64(b));
      push_r(r);
      release_r(b);
      break;
  }
}

void wasm_jit_t::emit_br_table() {
  uint32_t num_cases = read_u32leb(&buf_);
  std::map<uint32_t, size_t> dst_offsets;  // depth -> offset
  std::vector<uint32_t> table_offsets;     // table element -> offset
  inLocalLabel();
  int32_t a = pop_r(WASM_TYPE_I32);
  int32_t b = alloc_r(JIT_V_INTER, WASM_TYPE_I64);
  mov(to_xbyak_r32(b), num_cases);
  cmp(to_xbyak_r32(a), to_xbyak_r32(b));
  // use default case as the last one
  cmovae(to_xbyak_r32(a), to_xbyak_r32(b));
  size_t lea_pos = getSize();
  lea(to_xbyak_r64(b), qword[rip + 0x12345678]);
  size_t start_offset = getSize();
  movsxd(to_xbyak_r64(a), dword[to_xbyak_r64(b) + to_xbyak_r64(a) * 4]);
  add(to_xbyak_r64(a), to_xbyak_r64(b));
  db(0x3e);  // disable endbr64
  jmp(to_xbyak_r32(a));
  align(4);
  size_t end_offset = getSize();
  setSize(lea_pos);
  lea(to_xbyak_r64(b), qword[rip + (int)(end_offset - start_offset)]);
  if (getSize() != start_offset) {
    throw std::runtime_error("Generated wrong rip addressing LEA");
  }
  // alloc space for offsets
  table_offsets.resize(num_cases + 1);
  setSize(end_offset + 4 * (num_cases + 1));
  // also takes care of default case
  for (uint32_t i = 0; i <= num_cases; i++) {
    uint32_t depth = read_u32leb(&buf_);
    auto result = dst_offsets.find(depth);
    if (result == dst_offsets.end()) {
      dst_offsets[depth] = getSize();
      table_offsets[i] = getSize();
      block_t* b = (&block_stack_.back()) - depth;
      uint32_t t = b->br_ctxs.size();
      std::string jmp_label = "F" + std::to_string(funcidx_) + "L" +
                              std::to_string(b->index) + "T" +
                              std::to_string(t);
      jmp(jmp_label, T_NEAR);
      b->br_ctxs.push_back(ctx_);
    } else {
      table_offsets[i] = result->second;
    }
  }
  size_t inst_end_offset = getSize();
  setSize(end_offset);
  for (uint32_t i = 0; i <= num_cases; i++) {
    dd((uint32_t)(table_offsets[i] - end_offset));
  }
  setSize(inst_end_offset);
  release_r(a);
  release_r(b);
  outLocalLabel();
}
