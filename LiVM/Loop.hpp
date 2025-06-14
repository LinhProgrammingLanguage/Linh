#pragma once
#include "LiVM.hpp"
#include "../LinhC/Bytecode/Bytecode.hpp"

namespace Linh
{
    void handle_loop_opcode(LiVM &vm, const Instruction &instr, const BytecodeChunk &chunk, size_t &ip);
}
