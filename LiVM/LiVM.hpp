#pragma once
#include "../LinhC/Bytecode/Bytecode.hpp"
#include <vector>
#include <variant>
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

        struct Function
        {
            BytecodeChunk code;
            std::vector<std::string> param_names;
        };
        std::unordered_map<std::string, Function> functions;
        struct CallFrame
        {
            size_t return_ip;
            std::unordered_map<int, std::variant<std::monostate, int64_t, double, std::string, bool>> locals;
        };
        std::vector<CallFrame> call_stack;

        void set_functions(const std::unordered_map<std::string, Function> &funcs)
        {
            functions = funcs;
        }

    private:
        std::vector<std::variant<std::monostate, int64_t, double, std::string, bool>> stack;
        std::unordered_map<int, std::variant<std::monostate, int64_t, double, std::string, bool>> variables;
        size_t ip = 0; // instruction pointer

        void push(const std::variant<std::monostate, int64_t, double, std::string, bool> &val);
        std::variant<std::monostate, int64_t, double, std::string, bool> pop();
        std::variant<std::monostate, int64_t, double, std::string, bool> peek();
    };
}