#include "Loop.hpp"

namespace Linh
{
    void handle_loop_opcode(LiVM &vm, const Instruction &instr, const BytecodeChunk &chunk, size_t &ip)
    {
        switch (instr.opcode)
        {
        case OpCode::JMP:
            ip = std::get<int64_t>(instr.operand);
            break;
        case OpCode::JMP_IF_FALSE:
        {
            auto cond = vm.pop();
            bool cond_val = false;
            if (std::holds_alternative<bool>(cond))
                cond_val = std::get<bool>(cond);
            else if (std::holds_alternative<int64_t>(cond))
                cond_val = std::get<int64_t>(cond) != 0;
            else if (std::holds_alternative<double>(cond))
                cond_val = std::get<double>(cond) != 0.0;
            else if (std::holds_alternative<std::string>(cond))
                cond_val = !std::get<std::string>(cond).empty();
            if (!cond_val)
                ip = std::get<int64_t>(instr.operand);
            else
                ++ip;
            return;
        }
        case OpCode::JMP_IF_TRUE:
        {
            auto cond = vm.pop();
            bool cond_val = false;
            if (std::holds_alternative<bool>(cond))
                cond_val = std::get<bool>(cond);
            else if (std::holds_alternative<int64_t>(cond))
                cond_val = std::get<int64_t>(cond) != 0;
            else if (std::holds_alternative<double>(cond))
                cond_val = std::get<double>(cond) != 0.0;
            else if (std::holds_alternative<std::string>(cond))
                cond_val = !std::get<std::string>(cond).empty();
            if (cond_val)
                ip = std::get<int64_t>(instr.operand);
            else
                ++ip;
            return;
        }
        default:
            ++ip;
            break;
        }
    }
}
