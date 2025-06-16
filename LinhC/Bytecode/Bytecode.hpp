#pragma once
#include <vector>
#include <string>
#include <variant>
#include <cstdint>

namespace Linh
{
    enum class OpCode : uint8_t
    {
        // Basic stack ops
        NOP,
        PUSH_INT,
        PUSH_FLOAT,
        PUSH_STR,
        PUSH_BOOL,
        POP,
        SWAP, // <--- Add this for unary minus

        // Arithmetic
        ADD,
        SUB,
        MUL,
        DIV,
        MOD,

        // Logic
        AND,
        OR,
        NOT,

        // Comparison
        EQ,
        NEQ,
        LT,
        GT,
        LTE,
        GTE,

        // Variable ops
        LOAD_VAR,
        STORE_VAR,

        // Control flow
        JMP,
        JMP_IF_FALSE,
        JMP_IF_TRUE,

        // Function
        CALL,
        RET,

        // Special
        PRINT,
        INPUT,  // <--- Add this for input
        TYPEOF, // <--- Thêm dòng này
        HALT
    };

    using BytecodeValue = std::variant<int64_t, double, std::string, bool>;

    struct Instruction
    {
        OpCode opcode;
        BytecodeValue operand;
        int line = 0; // <--- Thêm dòng này
        int col = 0;  // <--- Thêm dòng này
        Instruction(OpCode op, BytecodeValue val = {}, int l = 0, int c = 0)
            : opcode(op), operand(std::move(val)), line(l), col(c) {}
    };

    using BytecodeChunk = std::vector<Instruction>;
}
