#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "weerun.hpp"
#include "weerun_jit.hpp"

template <typename T>
T decode(buffer_t* buf);

template <typename T>
std::vector<T> decode_vector(buffer_t* buf);

void parse_custom_section(wasm_module_t* module, buffer_t* buf);
void parse_type_section(wasm_module_t* module, buffer_t* buf);
void parse_import_section(wasm_module_t* module, buffer_t* buf);
void parse_function_section(wasm_module_t* module, buffer_t* buf);
void parse_table_section(wasm_module_t* module, buffer_t* buf);
void parse_memory_section(wasm_module_t* module, buffer_t* buf);
void parse_global_section(wasm_module_t* module, buffer_t* buf);
void parse_export_section(wasm_module_t* module, buffer_t* buf);
void parse_start_section(wasm_module_t* module, buffer_t* buf);
void parse_element_section(wasm_module_t* module, buffer_t* buf);
void parse_code_section(wasm_module_t* module, buffer_t* buf);
void parse_data_section(wasm_module_t* module, buffer_t* buf);

 wasm_module_t* parse(buffer_t* buf) {
  buf->ptr += 8;  // magic and version
  wasm_module_t* module = new wasm_module_t();
  module->num_import_funcs_ = 0;
  module->have_start_ = false;
  while (buf->ptr != buf->end) {
    byte type = read_u8(buf);
    switch (type) {
      case 0:
        parse_custom_section(module, buf);
        break;
      case WASM_SECT_TYPE:
        parse_type_section(module, buf);
        break;
      case WASM_SECT_IMPORT:
        parse_import_section(module, buf);
        break;
      case WASM_SECT_FUNCTION:
        parse_function_section(module, buf);
        break;
      case WASM_SECT_TABLE:
        parse_table_section(module, buf);
        break;
      case WASM_SECT_MEMORY:
        parse_memory_section(module, buf);
        break;
      case WASM_SECT_GLOBAL:
        parse_global_section(module, buf);
        break;
      case WASM_SECT_EXPORT:
        parse_export_section(module, buf);
        break;
      case WASM_SECT_START:
        parse_start_section(module, buf);
        break;
      case WASM_SECT_ELEMENT:
        parse_element_section(module, buf);
        break;
      case WASM_SECT_CODE:
        parse_code_section(module, buf);
        break;
      case WASM_SECT_DATA:
        parse_data_section(module, buf);
        break;
      default:
        assert(0);
    }
  }
  return module;
}

wasm_instance_t* active_instance;

uint32_t jit_enable = 0;
uint32_t jit_check = 1;

static uint64_t default_code_size = (1 << 20);

 wasm_typed_value_t run(const byte* start,
                                  const byte* end,
                                  uint32_t num_args,
                                  wasm_value_t* args) {
  buffer_t onstack_buf = {start, start, end};
  wasm_module_t* module = parse(&onstack_buf);
  wasm_instance_t* instance = module->create_instance();
  wasm_value_t result;
  if (jit_enable != 0) {
    result =
        instance->run_jit(std::vector<wasm_value_t>(args, args + num_args));
  } else {
    result = instance->run(std::vector<wasm_value_t>(args, args + num_args));
  }
  wasm_typed_value_t tr = {module->funcs_[module->main_index_].sig_->ret_[0],
                           result};
  delete instance;
  delete module;
  return tr;
}

void parse_custom_section(wasm_module_t* module, buffer_t* buf) {
  (void)module;
  uint32_t size = read_u32leb(buf);
  const byte* section_end = buf->ptr + size;
  std::string name = decode<std::string>(buf);
  buf->ptr = section_end;
  return;
}

void parse_type_section(wasm_module_t* module, buffer_t* buf) {
  read_u32leb(buf);
  module->sigs_ = decode_vector<wasm_func_decl_t>(buf);
}

void parse_import_section(wasm_module_t* module, buffer_t* buf) {
  read_u32leb(buf);
  std::vector<wasm_import_decl_t> imports =
      decode_vector<wasm_import_decl_t>(buf);
  // create imported functions
  for (wasm_import_decl_t& imp : imports) {
    switch (imp.type_) {
      case wasm_import_decl_t::type::FUNCTION: {
        std::string name = imp.module_ + "." + imp.name_;
        auto result = g_runtime_funcs.find(name);
        if (result == g_runtime_funcs.end()) {
          std::cout << "error: " << name << " cannot be imported";
          throw std::runtime_error(name + " not found");
        }
        wasm_func_t f;
        f.sig_ = &module->sigs_[imp.funcindex_];
        f.typeidx_ = imp.funcindex_;
        f.run_ = result->second.run_;
        f.run_jit_ = result->second.run_jit_;
        module->funcs_.push_back(f);
        module->num_import_funcs_++;
        break;
      }
      default:
        break;
    }
  }
}

void parse_function_section(wasm_module_t* module, buffer_t* buf) {
  read_u32leb(buf);
  std::vector<uint32_t> funcidxs = decode_vector<uint32_t>(buf);
  // create function objects
  for (uint32_t func : funcidxs) {
    wasm_func_t f;
    f.sig_ = &module->sigs_[func];
    f.typeidx_ = func;
    f.run_ = nullptr;
    module->funcs_.push_back(f);
  }
}

void parse_table_section(wasm_module_t* module, buffer_t* buf) {
  read_u32leb(buf);
  std::vector<wasm_table_decl_t> tables = decode_vector<wasm_table_decl_t>(buf);
  module->table_ = tables[0];
}

void parse_memory_section(wasm_module_t* module, buffer_t* buf) {
  read_u32leb(buf);
  std::vector<wasm_memory_decl_t> memories =
      decode_vector<wasm_memory_decl_t>(buf);
  module->mem_limits_ = memories[0];
}

void parse_global_section(wasm_module_t* module, buffer_t* buf) {
  read_u32leb(buf);
  module->globals_ = decode_vector<wasm_global_t>(buf);
}

void parse_export_section(wasm_module_t* module, buffer_t* buf) {
  read_u32leb(buf);
  std::vector<wasm_export_t> exps = decode_vector<wasm_export_t>(buf);
  // we assume that the module only exports a main function
  module->main_index_ = exps[0].idx_;
}

void parse_start_section(wasm_module_t* module, buffer_t* buf) {
  read_u32leb(buf);
  module->have_start_ = true;
  module->start_index_ = read_u32leb(buf);
}

void parse_element_section(wasm_module_t* module, buffer_t* buf) {
  read_u32leb(buf);
  module->elems_ = decode_vector<wasm_elem_decl_t>(buf);
}

std::vector<wasm_code_decl_t> decode_vector_code_decl(buffer_t* buf,
                                                      wasm_module_t* module);

void parse_code_section(wasm_module_t* module, buffer_t* buf) {
  read_u32leb(buf);
  std::vector<wasm_code_decl_t> codes = decode_vector_code_decl(buf, module);
  // start from the first non-imported functions
  uint32_t funcidx = module->num_import_funcs_;
  for (wasm_code_decl_t& code : codes) {
    module->funcs_[funcidx].locals_ = code.locals_;
    module->funcs_[funcidx].code_ = code.code_;
    funcidx++;
  }
}

void parse_data_section(wasm_module_t* module, buffer_t* buf) {
  read_u32leb(buf);
  module->datas_ = decode_vector<wasm_data_decl_t>(buf);
}

wasm_instance_t* wasm_module_t::create_instance() {
  wasm_instance_t* instance = new wasm_instance_t();
  instance->module_ = this;

  // initialize memory using data
  instance->memory_.resize(mem_limits_.init_ * wasm_page_size, 0);
  for (wasm_data_decl_t& data : datas_) {
    byte* mem = instance->memory_.data();
    std::memcpy(mem + data.offset_, data.data_.data(), data.data_.size());
  }

  // initialize global variables
  instance->globals_.resize(globals_.size());
  for (uint32_t i = 0; i < globals_.size(); i++) {
    switch (globals_[i].type_.type_) {
      case WASM_TYPE_I32:
        instance->globals_[i].i32 = globals_[i].val_.i32;
        break;
      case WASM_TYPE_F64:
        instance->globals_[i].f64 = globals_[i].val_.f64;
        break;
      default:
        instance->globals_[i].ref = globals_[i].val_.ref;
        break;
    }
  }

  // initialize table
  instance->table_.resize(table_.init_, nullptr);
  for (wasm_elem_decl_t& elem : elems_) {
    instance->ensure_table(elem.offset_ + elem.funcidx_.size());
    wasm_func_t** p = &instance->table_[elem.offset_];
    for (uint32_t funcidx : elem.funcidx_) {
      *p = &funcs_[funcidx];
      p++;
    }
  }
  instance->jit_table_.resize(instance->table_.size(), {-1, nullptr});
  return instance;
}

void wasm_instance_t::push_value(wasm_value_t v) {
  value_stack_.push_back(v);
}

wasm_value_t wasm_instance_t::pop_value() {
  wasm_value_t result = value_stack_.back();
  value_stack_.pop_back();
  return result;
}

void wasm_instance_t::branch(uint32_t to_off) {
  cur_frame_->pc_.ptr = cur_frame_->pc_.start + to_off;
}

void wasm_instance_t::push_frame(wasm_func_t* f) {
  if (f->run_ != nullptr) {
    // runtime functions
    f->run_(this);
    return;
  }

  // create a frame for function f
  wasm_stack_frame_t frame;
  frame.f_ = f;

  // locals contains arguments and f's own locals
  uint32_t num_args = f->sig_->args_.size();
  frame.locals_.resize(num_args);
  // move argument from stack
  for (int i = num_args - 1; i >= 0; i--) {
    frame.locals_[i] = pop_value();
  }
  // initialize locals
  for (wasm_local_decl_t& local : f->locals_) {
    wasm_value_t v;
    switch (local.type_) {
      case WASM_TYPE_I32:
        v.i32 = 0;
        break;
      case WASM_TYPE_F64:
        v.f64 = 0;
        break;
      case WASM_TYPE_EXTERNREF:
        v.ref = nullptr;
    }
    for (uint32_t i = 0; i < local.count_; i++) {
      frame.locals_.push_back(v);
    }
  }

  // set pc to the start of function bytecode
  frame.pc_.start = frame.pc_.ptr = f->code_.data();
  frame.pc_.end = frame.pc_.start + f->code_.size();
  frames_.push_back(frame);

  // update current frame
  cur_frame_ = &frames_.back();
}

void wasm_instance_t::pop_frame() {
  // pop the from when a funtcion reaches the end
  frames_.pop_back();
  cur_frame_ = &frames_.back();
}

wasm_value_t wasm_instance_t::run(std::vector<wasm_value_t> args) {
  if (module_->have_start_ == true) {
    run_function(module_->start_index_);
  }
  // create the frame for main()
  for (wasm_value_t& v : args) {
    value_stack_.push_back(v);
  }
  active_instance = this;
  run_function(module_->main_index_);
  // return the value on stack;
  return value_stack_.back();
}

wasm_value_t wasm_instance_t::run_jit(std::vector<wasm_value_t> args) {
  wasm_jit_t jit(default_code_size, jit_check != 0);  // 1M code size
  jit.compile_instance(this);

  active_instance = this;
  if (module_->have_start_ == true) {
    jit.run_fn(module_->start_index_, nullptr, nullptr);
  }
  wasm_value_t r;
  jit.run_fn(module_->main_index_, args.data(), &r);
  return r;
}

void wasm_instance_t::run_function(uint32_t funcidx) {
  trapped = WASM_TRAP_REASON_NOTRAP;
  push_frame(&module_->funcs_[funcidx]);

  // start interpretation
  while (trapped == WASM_TRAP_REASON_NOTRAP) {
    byte opcode = read_u8(&cur_frame_->pc_);
    const wasm_inst_desc_t* desc = &g_inst_desc[opcode];
    desc->run(this, opcode);
  }

  if (trapped != WASM_TRAP_REASON_FUNCEND) {
    std::cout << "!trap" << std::endl;
    std::exit(0);
  }
}

void wasm_instance_t::trap(uint32_t reason) {
  trapped = reason;
}

uint32_t wasm_instance_t::read_u32leb() {
  return ::read_u32leb(&cur_frame_->pc_);
}

int32_t wasm_instance_t::read_i32leb() {
  return ::read_i32leb(&cur_frame_->pc_);
}

double wasm_instance_t::read_double() {
  double result = *(double*)(cur_frame_->pc_.ptr);
  cur_frame_->pc_.ptr += sizeof(double);
  return result;
}

byte wasm_instance_t::read_byte() {
  byte result = *(byte*)(cur_frame_->pc_.ptr);
  cur_frame_->pc_.ptr += sizeof(byte);
  return result;
}

void wasm_instance_t::ensure_table(uint32_t offset) {
  if (table_.size() < offset) {
    table_.resize(offset, nullptr);
  }
}
