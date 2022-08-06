//
// Created by kevin on 8/6/22.
//

#include "common.h"
#include <iomanip>
#include <iostream>
#include <sstream>

void disassemble_chunk(Chunk const &chunk)
{
    uint64_t offset = 0;
    auto const number_of_instructions = chunk.byte_code.size();
    while (offset < number_of_instructions)
    {
        switch (chunk.byte_code[offset])
        {
        case OP_RETURN:
            std::cout << std::setfill('0') << std::setw(5) << offset << "OP_RETURN\n";
            ++offset;
            break;
        case OP_CONSTANT:
            std::cout << std::setfill('0') << std::setw(5) << offset << "OP_CONSTANT " << chunk.byte_code[offset + 1]
                      << "\n";
            offset += 2;
            break;
        default:
            std::cout << "Unknown op code\n";
            break;
        }
    }
}