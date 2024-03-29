#include <cmath>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "weerun.hpp"

void trap();

std::unordered_map<std::string, wasm_rt_func_t> g_runtime_funcs;

static void weewasm_puti(wasm_instance_t* ins) {
  wasm_value_t v = ins->pop_value();
  std::printf("%d", (int32_t)v.val.i32);
}

static void weewasm_putd(wasm_instance_t* ins) {
  wasm_value_t v = ins->pop_value();
  std::printf("%f", v.val.f64);
}

static void weewasm_puts(wasm_instance_t* ins) {
  wasm_value_t size = ins->pop_value();
  wasm_value_t start = ins->pop_value();
  const byte* base = ins->memory_.data() + start.val.i32;
  std::cout << std::string(base, base + size.val.i32);
}

static void weewasm_puti_jit(int32_t v) {
  std::printf("%d", v);
}

static void weewasm_putd_jit(double v) {
  std::printf("%f", v);
}

static void weewasm_puts_jit(uint32_t start, uint32_t size) {
  const byte* base = active_instance->memory_.data() + start;
  std::cout << std::string(base, base + size);
}

static inline wasm_key_t convert_key(wasm_externref_t* ref) {
  union {
    uint64_t i64;
    double f64;
    void* ptr;
  } u;
  if (ref == nullptr) {
    return {WASM_EXTERN_NULL, 0};
  }
  switch (ref->type_) {
    case WASM_EXTERN_BOXI32:
      u.i64 = ((wasm_e_box_t*)ref)->i32_;
      break;
    case WASM_EXTERN_BOXF64:
      u.f64 = ((wasm_e_box_t*)ref)->f64_;
      break;
    default:
      u.ptr = (void*)ref;
      break;
  }
  return {ref->type_, u.i64};
}

static void weewasm_obj_new(wasm_instance_t* ins) {
  wasm_value_t v;
  v.val.ref = (void*)(new wasm_e_object_t());
  ins->push_value(v);
}

static void weewasm_obj_box_i32(wasm_instance_t* ins) {
  wasm_e_box_t* obj = new wasm_e_box_t(WASM_EXTERN_BOXI32);
  wasm_value_t x = ins->pop_value();
  obj->i32_ = (int32_t)x.val.i32;
  wasm_value_t v;
  v.val.ref = (void*)(obj);
  ins->push_value(v);
}

static void weewasm_obj_box_f64(wasm_instance_t* ins) {
  wasm_e_box_t* obj = new wasm_e_box_t(WASM_EXTERN_BOXF64);
  wasm_value_t x = ins->pop_value();
  obj->f64_ = x.val.f64;
  wasm_value_t v;
  v.val.ref = (void*)(obj);
  ins->push_value(v);
}

static void weewasm_obj_get(wasm_instance_t* ins) {
  wasm_value_t key = ins->pop_value();
  wasm_value_t o = ins->pop_value();
  if (key.val.ref == nullptr || o.val.ref == nullptr) {
    ins->trap(WASM_TRAP_REASON_NULLEXTERNREF);
    return;
  }
  wasm_e_object_t* obj = (wasm_e_object_t*)o.val.ref;
  if (obj->type_ != WASM_EXTERN_OBJECT) {
    ins->trap(WASM_TRAP_REASON_BADOBJECT);
    return;
  }
  wasm_value_t v;
  auto result = obj->map_.find(convert_key((wasm_externref_t*)key.val.ref));
  if (result == obj->map_.end()) {
    v.val.ref = nullptr;
  } else {
    v.val.ref = (void*)result->second;
  }
  ins->push_value(v);
  return;
}

static void weewasm_obj_set(wasm_instance_t* ins) {
  wasm_value_t value = ins->pop_value();
  wasm_value_t key = ins->pop_value();
  wasm_value_t o = ins->pop_value();
  if (key.val.ref == nullptr || o.val.ref == nullptr) {
    ins->trap(WASM_TRAP_REASON_NULLEXTERNREF);
    return;
  }
  wasm_e_object_t* obj = (wasm_e_object_t*)o.val.ref;
  if (obj->type_ != WASM_EXTERN_OBJECT) {
    ins->trap(WASM_TRAP_REASON_BADOBJECT);
    return;
  }
  obj->map_[convert_key((wasm_externref_t*)key.val.ref)] =
      (wasm_externref_t*)value.val.ref;
  return;
}

static void weewasm_i32_unbox(wasm_instance_t* ins) {
  wasm_value_t value = ins->pop_value();
  wasm_e_box_t* box = (wasm_e_box_t*)value.val.ref;
  if (box == nullptr || box->type_ != WASM_EXTERN_BOXI32) {
    ins->trap(WASM_TRAP_REASON_BADOBJECT);
    return;
  }
  wasm_value_t v;
  v.val.i32 = (uint32_t)box->i32_;
  ins->push_value(v);
  return;
}

static void weewasm_f64_unbox(wasm_instance_t* ins) {
  wasm_value_t value = ins->pop_value();
  wasm_e_box_t* box = (wasm_e_box_t*)value.val.ref;
  if (box == nullptr || box->type_ != WASM_EXTERN_BOXF64) {
    ins->trap(WASM_TRAP_REASON_BADOBJECT);
    return;
  }
  wasm_value_t v;
  v.val.f64 = box->f64_;
  ins->push_value(v);
  return;
}

static void weewasm_obj_eq(wasm_instance_t* ins) {
  wasm_value_t rhs = ins->pop_value();
  wasm_value_t lhs = ins->pop_value();
  wasm_value_t v;
  v.val.i32 = (convert_key((wasm_externref_t*)lhs.val.ref) ==
               convert_key((wasm_externref_t*)rhs.val.ref))
                  ? 1
                  : 0;
  ins->push_value(v);
}

static wasm_e_object_t* weewasm_obj_new_jit() {
  return new wasm_e_object_t();
}

static wasm_e_box_t* weewasm_obj_box_i32_jit(int32_t v) {
  wasm_e_box_t* obj = new wasm_e_box_t(WASM_EXTERN_BOXI32);
  obj->i32_ = v;
  return obj;
}

static wasm_e_box_t* weewasm_obj_box_f64_jit(double v) {
  wasm_e_box_t* obj = new wasm_e_box_t(WASM_EXTERN_BOXF64);
  obj->f64_ = v;
  return obj;
}

static wasm_externref_t* weewasm_obj_get_jit(wasm_e_object_t* obj,
                                             wasm_externref_t* key) {
  if (key == nullptr || obj == nullptr) {
    trap();
  }
  if (obj->type_ != WASM_EXTERN_OBJECT) {
    trap();
  }
  auto result = obj->map_.find(convert_key(key));
  if (result == obj->map_.end()) {
    return nullptr;
  }
  return result->second;
}

static void weewasm_obj_set_jit(wasm_e_object_t* obj,
                                wasm_externref_t* key,
                                wasm_externref_t* value) {
  if (key == nullptr || obj == nullptr) {
    trap();
  }
  if (obj->type_ != WASM_EXTERN_OBJECT) {
    trap();
  }
  obj->map_[convert_key(key)] = value;
}

static int32_t weewasm_i32_unbox_jit(wasm_e_box_t* box) {
  if (box == nullptr || box->type_ != WASM_EXTERN_BOXI32) {
    trap();
  }
  return box->i32_;
}

static double weewasm_f64_unbox_jit(wasm_e_box_t* box) {
  if (box == nullptr || box->type_ != WASM_EXTERN_BOXF64) {
    trap();
  }
  return box->f64_;
}

static uint32_t weewasm_obj_eq_jit(wasm_externref_t* lhs,
                                   wasm_externref_t* rhs) {
  return (convert_key(lhs) == convert_key(rhs)) ? 1 : 0;
}

extern "C" void init_runtime() {
  g_runtime_funcs["weewasm.puti"] = {weewasm_puti, (void*)weewasm_puti_jit};
  g_runtime_funcs["weewasm.putd"] = {weewasm_putd, (void*)weewasm_putd_jit};
  g_runtime_funcs["weewasm.puts"] = {weewasm_puts, (void*)weewasm_puts_jit};
  g_runtime_funcs["weewasm.obj.new"] = {weewasm_obj_new,
                                        (void*)weewasm_obj_new_jit};
  g_runtime_funcs["weewasm.obj.box_i32"] = {weewasm_obj_box_i32,
                                            (void*)weewasm_obj_box_i32_jit};
  g_runtime_funcs["weewasm.obj.box_f64"] = {weewasm_obj_box_f64,
                                            (void*)weewasm_obj_box_f64_jit};
  g_runtime_funcs["weewasm.obj.get"] = {weewasm_obj_get,
                                        (void*)weewasm_obj_get_jit};
  g_runtime_funcs["weewasm.obj.set"] = {weewasm_obj_set,
                                        (void*)weewasm_obj_set_jit};
  g_runtime_funcs["weewasm.i32.unbox"] = {weewasm_i32_unbox,
                                          (void*)weewasm_i32_unbox_jit};
  g_runtime_funcs["weewasm.f64.unbox"] = {weewasm_f64_unbox,
                                          (void*)weewasm_f64_unbox_jit};
  g_runtime_funcs["weewasm.obj.eq"] = {weewasm_obj_eq,
                                       (void*)weewasm_obj_eq_jit};
}

static void libc_sin(wasm_instance_t* ins) {
  wasm_value_t x = ins->pop_value();
  x.val.f64 = std::sin(x.val.f64);
  ins->push_value(x);
}

static void libc_cos(wasm_instance_t* ins) {
  wasm_value_t x = ins->pop_value();
  x.val.f64 = std::cos(x.val.f64);
  ins->push_value(x);
}

static void libc_rand(wasm_instance_t* ins) {
  wasm_value_t x;
  x.val.i32 = std::rand();
  ins->push_value(x);
}

static void libc_time(wasm_instance_t* ins) {
  wasm_value_t x;
  x.val.i32 = std::time(nullptr);
  ins->push_value(x);
}

static void libc_srand(wasm_instance_t* ins) {
  wasm_value_t x = ins->pop_value();
  std::srand(x.val.i32);
}

int32_t libc_time_jit() {
  return std::time(nullptr);
}

extern "C" void init_libc_runtime() {
  static std::vector<wasm_func_decl_t> func_decls;
  func_decls.push_back({{WASM_TYPE_F64}, {WASM_TYPE_F64}});
  func_decls.push_back({{}, {WASM_TYPE_I32}});
  func_decls.push_back({{WASM_TYPE_I32}, {}});
  g_runtime_funcs["libc.sin"] = {
      libc_sin, (void*)static_cast<double (*)(double)>(std::sin)};
  g_runtime_funcs["libc.cos"] = {
      libc_cos, (void*)static_cast<double (*)(double)>(std::cos)};
  g_runtime_funcs["libc.rand"] = {libc_rand, (void*)std::rand};
  g_runtime_funcs["libc.time"] = {libc_time, (void*)libc_time_jit};
  g_runtime_funcs["libc.srand"] = {libc_srand, (void*)std::srand};
}
