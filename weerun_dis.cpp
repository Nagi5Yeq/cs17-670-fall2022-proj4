#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "weerun.hpp"

template <typename T>
T decode(buffer_t* buf);

template <typename T>
std::vector<T> decode_vector(buffer_t* buf) {
  std::vector<T> result;
  uint32_t num_elem = read_u32leb(buf);
  for (uint32_t i = 0; i < num_elem; i++) {
    result.push_back(decode<T>(buf));
  }
  return result;
}

template <>
byte decode<byte>(buffer_t* buf) {
  return read_u8(buf);
}

template <>
uint32_t decode<uint32_t>(buffer_t* buf) {
  return read_u32leb(buf);
}

template <>
std::string decode<std::string>(buffer_t* buf) {
  uint32_t length = read_u32leb(buf);
  std::string result(buf->ptr, buf->ptr + length);
  buf->ptr += length;
  return result;
}

template <>
wasm_func_decl_t decode<wasm_func_decl_t>(buffer_t* buf) {
  read_u8(buf);
  wasm_func_decl_t result;
  result.args_ = decode_vector<uint8_t>(buf);
  result.ret_ = decode_vector<uint8_t>(buf);
  return result;
}

template <>
wasm_table_decl_t decode<wasm_table_decl_t>(buffer_t* buf) {
  wasm_table_decl_t result;
  result.type_ = read_u8(buf);
  result.has_max_ = read_u8(buf);
  result.init_ = read_u32leb(buf);
  if (result.has_max_ != 0) {
    result.max_ = read_u32leb(buf);
  }
  return result;
}

template <>
wasm_memory_decl_t decode<wasm_memory_decl_t>(buffer_t* buf) {
  wasm_memory_decl_t result;
  result.has_max_ = read_u8(buf);
  result.init_ = read_u32leb(buf);
  if (result.has_max_ != 0) {
    result.max_ = read_u32leb(buf);
  }
  return result;
}

template <>
wasm_global_decl_t decode<wasm_global_decl_t>(buffer_t* buf) {
  wasm_global_decl_t result;
  result.type_ = read_u8(buf);
  result.is_mut_ = read_u8(buf);
  return result;
}

template <>
wasm_import_decl_t decode<wasm_import_decl_t>(buffer_t* buf) {
  wasm_import_decl_t result;
  result.module_ = decode<std::string>(buf);
  result.name_ = decode<std::string>(buf);
  result.type_ = (wasm_import_decl_t::type)read_u8(buf);
  switch (result.type_) {
    case wasm_import_decl_t::type::FUNCTION:
      result.funcindex_ = read_u32leb(buf);
      break;
    default:
      break;
  }
  return result;
}

template <>
wasm_global_t decode<wasm_global_t>(buffer_t* buf) {
  wasm_global_t result;
  result.type_ = decode<wasm_global_decl_t>(buf);
  uint32_t opcode = read_u8(buf);
  switch (opcode) {
    case WASM_OP_I32_CONST:
      result.val_.i32 = read_i32leb(buf);
      break;
    case WASM_OP_F64_CONST:
      result.val_.f64 = *(double*)buf->ptr;
      buf->ptr += sizeof(double);
      break;
  }
  opcode = read_u8(buf);
  return result;
}

template <>
wasm_export_t decode<wasm_export_t>(buffer_t* buf) {
  wasm_export_t result;
  result.name_ = decode<std::string>(buf);
  result.type_ = read_u32leb(buf);
  result.idx_ = read_u32leb(buf);
  return result;
}

template <>
wasm_elem_decl_t decode<wasm_elem_decl_t>(buffer_t* buf) {
  wasm_elem_decl_t result;
  uint32_t tmp = read_u32leb(buf);
  assert(tmp == 0);
  tmp = read_u8(buf);
  assert(tmp == WASM_OP_I32_CONST);
  result.offset_ = read_i32leb(buf);
  tmp = read_u8(buf);
  assert(tmp == WASM_OP_END);
  result.funcidx_ = decode_vector<uint32_t>(buf);
  return result;
}

template <>
wasm_local_decl_t decode<wasm_local_decl_t>(buffer_t* buf) {
  wasm_local_decl_t result;
  result.count_ = read_u32leb(buf);
  result.type_ = read_u8(buf);
  return result;
}

wasm_code_decl_t decode_code_decl(buffer_t* buf, wasm_module_t* module) {
  wasm_code_decl_t result;
  result.size_ = read_u32leb(buf);
  result.locals_ = decode_vector<wasm_local_decl_t>(buf);
  const byte* code_start = buf->ptr;

  int blocks = 1;
  //  a block's enclosing block
  std::map<const byte*, const byte*> block_parent;

  // a br's target block when br happens
  std::map<const byte*, const byte*> br_target;

  // when we reach an OP_END, we can record the current block's end/start
  // address. for OP_BLOCK we record the end address (OP_END+1). for OP_LOOP we
  // record the start address (OP_LOOP+1). so when a br instruction wants to
  // jump to this block, we know where to jump
  std::map<const byte*, const byte*> block_target;

  const byte* cur_block = code_start;
  block_parent[cur_block] = cur_block;
  while (blocks != 0) {
    uint32_t opcode = read_u8(buf);
    const wasm_inst_desc_t* desc = &g_inst_desc[opcode];
    switch (desc->type_) {
      case WASM_INST_TYPE_NOARG:
        if (opcode == WASM_OP_END) {
          blocks--;
          if (blocks == 0) {
            // reach the OP_END for the function body
            block_target[cur_block] = buf->ptr - 1;
            break;
          }
          // An OP_END for an OP_BLOCK/OP_LOOP
          if (*(cur_block - 2) == WASM_OP_BLOCK) {
            // br a block will go to the end
            block_target[cur_block] = buf->ptr;
          } else {
            // br a loop will go to the beginning (skip OP_LOOP and blocktype)
            block_target[cur_block] = cur_block;
          }
          cur_block = block_parent[cur_block];
        }
        break;
      case WASM_INST_TYPE_I32:
        if (opcode == WASM_OP_BR || opcode == WASM_OP_BR_IF) {
          const byte* ptr = buf->ptr;
          const byte* block = cur_block;
          uint32_t depth = read_u32leb(buf);
          while (depth > 0) {
            block = block_parent[block];
            depth--;
          }
          br_target[ptr] = block;
        } else {
          read_u32leb(buf);
        }
        break;
      case WASM_INST_TYPE_S32:
        read_i32leb(buf);
        break;
      case WASM_INST_TYPE_I32_I32:
        read_u32leb(buf);
        read_u32leb(buf);
        break;
      case WASM_INST_TYPE_F64:
        buf->ptr += sizeof(double);
        break;
      case WASM_INST_TYPE_MEMARG:
        read_u32leb(buf);
        read_u32leb(buf);
        break;
      case WASM_INST_TYPE_BLOCK:
        read_u8(buf);
        if (opcode == WASM_OP_BLOCK || opcode == WASM_OP_LOOP) {
          blocks++;
          // link a block to its enclosing block
          block_parent[buf->ptr] = cur_block;
          cur_block = buf->ptr;
        }
        break;
      case WASM_INST_TYPE_BRTABLE: {
        uint32_t num_cases = read_u32leb(buf);
        // cases and the default case
        for (uint32_t i = 0; i <= num_cases; i++) {
          const byte* ptr = buf->ptr;
          const byte* block = cur_block;
          // go through the blocks to find the target block
          uint32_t depth = read_u32leb(buf);
          while (depth > 0) {
            block = block_parent[block];
            depth--;
          }
          br_target[ptr] = block;
        }
      } break;
      default:
        break;
    }
  }
  result.code_ = std::vector<byte>(code_start, buf->ptr);
  byte* patch_code = result.code_.data();
  // resolve all brs and rewrite it to offset
  for (auto br : br_target) {
    *(uint32_t*)(patch_code + (br.first - code_start)) =
        (uint32_t)(block_target[br.second] - code_start);
  }
  return result;
}

wasm_code_decl_t decode_code_decl_jit(buffer_t* buf, wasm_module_t* module) {
  wasm_code_decl_t result;
  result.size_ = read_u32leb(buf);
  const byte* code_end = buf->ptr + result.size_;
  result.locals_ = decode_vector<wasm_local_decl_t>(buf);
  result.code_ = std::vector<byte>(buf->ptr, code_end);
  buf->ptr = code_end;
  return result;
}

// turns out that decoding code section needs the module context
std::vector<wasm_code_decl_t> decode_vector_code_decl(buffer_t* buf,
                                                      wasm_module_t* module) {
  std::vector<wasm_code_decl_t> result;
  uint32_t num_elem = read_u32leb(buf);
  for (uint32_t i = 0; i < num_elem; i++) {
    if (jit != 0) {
      result.push_back(decode_code_decl_jit(buf, module));
    } else {
      result.push_back(decode_code_decl(buf, module));
    }
  }
  return result;
}

template <>
wasm_data_decl_t decode<wasm_data_decl_t>(buffer_t* buf) {
  wasm_data_decl_t result;
  read_u32leb(buf);
  read_u8(buf);
  result.offset_ = read_i32leb(buf);
  read_u8(buf);
  result.data_ = decode_vector<byte>(buf);
  return result;
}

template std::vector<wasm_func_decl_t> decode_vector<wasm_func_decl_t>(
    buffer_t* buffer);
template std::vector<wasm_import_decl_t> decode_vector<wasm_import_decl_t>(
    buffer_t* buffer);
template std::vector<wasm_table_decl_t> decode_vector<wasm_table_decl_t>(
    buffer_t* buffer);
template std::vector<wasm_memory_decl_t> decode_vector<wasm_memory_decl_t>(
    buffer_t* buffer);
template std::vector<wasm_global_t> decode_vector<wasm_global_t>(
    buffer_t* buffer);
template std::vector<wasm_export_t> decode_vector<wasm_export_t>(
    buffer_t* buffer);
template std::vector<wasm_elem_decl_t> decode_vector<wasm_elem_decl_t>(
    buffer_t* buffer);
template std::vector<wasm_data_decl_t> decode_vector<wasm_data_decl_t>(
    buffer_t* buffer);
template std::vector<uint32_t> decode_vector<uint32_t>(buffer_t* buffer);
