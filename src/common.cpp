//
// Created by kevin on 8/6/22.
//

#include "common.h"
#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

void Disassemble_chunk(Chunk const &chunk) {
  uint64_t offset = 0;
  auto const number_of_instructions = chunk.byte_code.size();
  while (offset < number_of_instructions) {
    offset = Disassemble_instruction(chunk, offset);
  }
}

uint64_t Disassemble_instruction(Chunk const &chunk, uint64_t offset) {
  switch (chunk.byte_code[offset]) {
  case OP_RETURN:
    std::cout << std::setfill('0') << std::setw(5) << offset << "OP_RETURN\n";
    return ++offset;
  case OP_CONSTANT:
    std::cout << std::setfill('0') << std::setw(5) << offset << "OP_CONSTANT "
              << chunk.byte_code[offset + 1] << "\n";
    offset += 2;
    return offset;
  default:
    std::cerr << "Unknown op code\n";
    __builtin_trap();
  }
}