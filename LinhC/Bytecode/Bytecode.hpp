#pragma once
#include <vector>
#include <string>
#include <variant>
#include <cstdint>
#include <tuple>
#include <utility>

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
        DUP,  // <--- Thêm dòng này cho switch-case

        // Arithmetic
        ADD,
        SUB,
        MUL,
        DIV,
        MOD,
        HASH, // <-- This is the correct opcode for #

        // --- Bitwise ---
        AMP,   // &
        PIPE,  // |
        CARET, // ^
        LT_LT, // <<
        GT_GT, // >>

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
        INPUT,
        TYPEOF,
        HALT,

        // Array/Map
        PUSH_ARRAY,
        PUSH_MAP,
        ARRAY_GET,
        ARRAY_SET,
        MAP_GET,
        MAP_SET,
        ARRAY_LEN,
        ARRAY_APPEND,
        MAP_KEYS,
        MAP_VALUES,

        // --- Thêm cho try-catch-finally ---
        TRY,
        END_TRY
    };

    using BytecodeValue = std::variant<
        int64_t,
        double,
        std::string,
        bool,
        std::tuple<int64_t, int64_t, int64_t, std::string> // <--- Thêm dòng này
        >;

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
