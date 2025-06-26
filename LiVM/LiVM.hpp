#pragma once
#include "../LinhC/Bytecode/Bytecode.hpp"
#include "Value/Value.hpp"
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>

namespace Linh
{
    class LiVM
    {
    public:
        LiVM();
        void run(const BytecodeChunk &chunk);

        void type();

        friend void handle_loop_opcode(LiVM &vm, const Instruction &instr, const BytecodeChunk &chunk, size_t &ip);
        friend void math_binary_op(LiVM &vm, const Instruction &instr); // Thêm dòng này

        struct Function
        {
            BytecodeChunk code;
            std::vector<std::string> param_names;
        };
        std::unordered_map<std::string, Function> functions;
        struct CallFrame
        {
            size_t return_ip;
            std::unordered_map<int, Value> locals;
        };
        std::vector<CallFrame> call_stack;

        void set_functions(const std::unordered_map<std::string, Function> &funcs)
        {
            functions = funcs;
        }

    private:
        std::vector<Value> stack;
        std::unordered_map<int, Value> variables;
        size_t ip = 0; // instruction pointer

        void push(const Value &val);
        Value pop();
        Value peek();
    };
}