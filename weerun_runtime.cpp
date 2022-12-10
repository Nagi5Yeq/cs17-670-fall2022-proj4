#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "weerun.hpp"
#include "weerun_jit.hpp"

std::unordered_map<std::string, wasm_rt_func_t> g_runtime_funcs;

static void weewasm_puti(wasm_instance_t* ins) {
  wasm_value_t v = ins->pop_value();
  std::printf("%d", (int32_t)v.i32);
}

static void weewasm_putd(wasm_instance_t* ins) {
  wasm_value_t v = ins->pop_value();
  std::printf("%f", v.f64);
}

static void weewasm_puts(wasm_instance_t* ins) {
  wasm_value_t size = ins->pop_value();
  wasm_value_t start = ins->pop_value();
  const byte* base = ins->memory_.data() + start.i32;
  std::cout << std::string(base, base + size.i32);
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
  v.ref = (void*)(new wasm_e_object_t());
  ins->push_value(v);
}

static void weewasm_obj_box_i32(wasm_instance_t* ins) {
  wasm_e_box_t* obj = new wasm_e_box_t(WASM_EXTERN_BOXI32);
  wasm_value_t x = ins->pop_value();
  obj->i32_ = (int32_t)x.i32;
  wasm_value_t v;
  v.ref = (void*)(obj);
  ins->push_value(v);
}

static void weewasm_obj_box_f64(wasm_instance_t* ins) {
  wasm_e_box_t* obj = new wasm_e_box_t(WASM_EXTERN_BOXF64);
  wasm_value_t x = ins->pop_value();
  obj->f64_ = x.f64;
  wasm_value_t v;
  v.ref = (void*)(obj);
  ins->push_value(v);
}

static void weewasm_obj_get(wasm_instance_t* ins) {
  wasm_value_t key = ins->pop_value();
  wasm_value_t o = ins->pop_value();
  if (key.ref == nullptr || o.ref == nullptr) {
    ins->trap(WASM_TRAP_REASON_NULLEXTERNREF);
    return;
  }
  wasm_e_object_t* obj = (wasm_e_object_t*)o.ref;
  if (obj->type_ != WASM_EXTERN_OBJECT) {
    ins->trap(WASM_TRAP_REASON_BADOBJECT);
    return;
  }
  wasm_value_t v;
  auto result = obj->map_.find(convert_key((wasm_externref_t*)key.ref));
  if (result == obj->map_.end()) {
    v.ref = nullptr;
  } else {
    v.ref = (void*)result->second;
  }
  ins->push_value(v);
  return;
}

static void weewasm_obj_set(wasm_instance_t* ins) {
  wasm_value_t value = ins->pop_value();
  wasm_value_t key = ins->pop_value();
  wasm_value_t o = ins->pop_value();
  if (key.ref == nullptr || o.ref == nullptr) {
    ins->trap(WASM_TRAP_REASON_NULLEXTERNREF);
    return;
  }
  wasm_e_object_t* obj = (wasm_e_object_t*)o.ref;
  if (obj->type_ != WASM_EXTERN_OBJECT) {
    ins->trap(WASM_TRAP_REASON_BADOBJECT);
    return;
  }
  obj->map_[convert_key((wasm_externref_t*)key.ref)] =
      (wasm_externref_t*)value.ref;
  return;
}

static void weewasm_i32_unbox(wasm_instance_t* ins) {
  wasm_value_t value = ins->pop_value();
  wasm_e_box_t* box = (wasm_e_box_t*)value.ref;
  if (box == nullptr || box->type_ != WASM_EXTERN_BOXI32) {
    ins->trap(WASM_TRAP_REASON_BADOBJECT);
    return;
  }
  wasm_value_t v;
  v.i32 = (uint32_t)box->i32_;
  ins->push_value(v);
  return;
}

static void weewasm_f64_unbox(wasm_instance_t* ins) {
  wasm_value_t value = ins->pop_value();
  wasm_e_box_t* box = (wasm_e_box_t*)value.ref;
  if (box == nullptr || box->type_ != WASM_EXTERN_BOXF64) {
    ins->trap(WASM_TRAP_REASON_BADOBJECT);
    return;
  }
  wasm_value_t v;
  v.f64 = box->f64_;
  ins->push_value(v);
  return;
}

static void weewasm_obj_eq(wasm_instance_t* ins) {
  wasm_value_t rhs = ins->pop_value();
  wasm_value_t lhs = ins->pop_value();
  wasm_value_t v;
  v.i32 = (convert_key((wasm_externref_t*)lhs.ref) ==
           convert_key((wasm_externref_t*)rhs.ref))
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
    jit_trap();
  }
  if (obj->type_ != WASM_EXTERN_OBJECT) {
    jit_trap();
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
    jit_trap();
  }
  if (obj->type_ != WASM_EXTERN_OBJECT) {
    jit_trap();
  }
  obj->map_[convert_key(key)] = value;
}

static int32_t weewasm_i32_unbox_jit(wasm_e_box_t* box) {
  if (box == nullptr || box->type_ != WASM_EXTERN_BOXI32) {
    jit_trap();
  }
  return box->i32_;
}

static double weewasm_f64_unbox_jit(wasm_e_box_t* box) {
  if (box == nullptr || box->type_ != WASM_EXTERN_BOXF64) {
    jit_trap();
  }
  return box->f64_;
}

static uint32_t weewasm_obj_eq_jit(wasm_externref_t* lhs,
                                   wasm_externref_t* rhs) {
  return (convert_key(lhs) == convert_key(rhs)) ? 1 : 0;
}

void init_runtime() {
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

static void env_sin(wasm_instance_t* ins) {
  wasm_value_t x = ins->pop_value();
  x.f64 = std::sin(x.f64);
  ins->push_value(x);
}

static void env_cos(wasm_instance_t* ins) {
  wasm_value_t x = ins->pop_value();
  x.f64 = std::cos(x.f64);
  ins->push_value(x);
}

static void env_rand(wasm_instance_t* ins) {
  wasm_value_t x;
  x.i32 = std::rand();
  ins->push_value(x);
}

static void env_time(wasm_instance_t* ins) {
  wasm_value_t x;
  x.i32 = std::time(nullptr);
  ins->push_value(x);
}

static void env_srand(wasm_instance_t* ins) {
  wasm_value_t x = ins->pop_value();
  std::srand(x.i32);
}

int32_t env_time_jit() {
  return std::time(nullptr);
}

int32_t env_memcmp_jit(uint32_t s1, uint32_t s2, size_t n) {
  byte* base = active_instance->memory_.data();
  return std::memcmp(base + s1, base + s2, n);
}

uint32_t env_memcpy_jit(uint32_t dst, uint32_t src, size_t n) {
  byte* base = active_instance->memory_.data();
  void* r = std::memcpy(base + dst, base + src, n);
  return (byte*)r - base;
}

uint32_t env_memmove_jit(uint32_t dst, uint32_t src, size_t n) {
  byte* base = active_instance->memory_.data();
  void* r = std::memmove(base + dst, base + src, n);
  return (byte*)r - base;
}

uint32_t env_memset_jit(uint32_t s, int c, size_t n) {
  byte* base = active_instance->memory_.data();
  void* r = std::memset(base + s, c, n);
  return (byte*)r - base;
}

struct wasm_memory_base_t {
  byte* base;

  wasm_memory_base_t() : base(active_instance->memory_.data()) {}

  template <typename T>
  T get(uint32_t offset) {
    return (T)(base + offset);
  }
};

static std::map<uint32_t, FILE*> opened_file = {{0, stdin},
                                                {1, stdout},
                                                {2, stderr}};
static uint32_t next_fd = 3;

uint32_t env_fopen_jit(uint32_t filename, uint32_t modes) {
  wasm_memory_base_t base;
  FILE* f =
      std::fopen(base.get<const char*>(filename), base.get<const char*>(modes));
  opened_file[next_fd] = f;
  return next_fd++;
}

size_t env_fread_jit(uint32_t ptr, size_t size, size_t n, uint32_t stream) {
  wasm_memory_base_t base;
  return std::fread(base.get<void*>(ptr), size, n, opened_file[stream]);
}

size_t env_fwrite_jit(uint32_t ptr, size_t size, size_t n, uint32_t stream) {
  wasm_memory_base_t base;
  return std::fwrite(base.get<void*>(ptr), size, n, opened_file[stream]);
}

int env_fclose_jit(uint32_t stream) {
  return std::fclose(opened_file[stream]);
}

uint32_t env_alloc_memory_jit(size_t size) {
  size = ((size + 15) / 16) * 16;
  byte* r = active_instance->heap_end_;
  active_instance->heap_end_ += size;
  return r - active_instance->memory_.data();
}

void init_env_runtime() {
  g_runtime_funcs["env.sin"] = {
      env_sin, (void*)static_cast<double (*)(double)>(std::sin)};
  g_runtime_funcs["env.cos"] = {
      env_cos, (void*)static_cast<double (*)(double)>(std::cos)};
  g_runtime_funcs["env.rand"] = {env_rand, (void*)std::rand};
  g_runtime_funcs["env.time"] = {env_time, (void*)env_time_jit};
  g_runtime_funcs["env.srand"] = {env_srand, (void*)std::srand};
  g_runtime_funcs["env.memcmp"] = {nullptr, (void*)env_memcmp_jit};
  g_runtime_funcs["env.memcpy"] = {nullptr, (void*)env_memcpy_jit};
  g_runtime_funcs["env.memmove"] = {nullptr, (void*)env_memmove_jit};
  g_runtime_funcs["env.memset"] = {nullptr, (void*)env_memset_jit};
  g_runtime_funcs["env.fopen"] = {nullptr, (void*)env_fopen_jit};
  g_runtime_funcs["env.fread"] = {nullptr, (void*)env_fread_jit};
  g_runtime_funcs["env.fwrite"] = {nullptr, (void*)env_fwrite_jit};
  g_runtime_funcs["env.fclose"] = {nullptr, (void*)env_fclose_jit};
  g_runtime_funcs["env.alloc_memory"] = {nullptr, (void*)env_alloc_memory_jit};
}
