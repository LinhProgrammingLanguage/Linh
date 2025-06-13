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
        HALT
    };

    using BytecodeValue = std::variant<int64_t, double, std::string, bool>;

    struct Instruction
    {
        OpCode opcode;
        BytecodeValue operand;
        Instruction(OpCode op, BytecodeValue val = {}) : opcode(op), operand(std::move(val)) {}
    };

    using BytecodeChunk = std::vector<Instruction>;
}
