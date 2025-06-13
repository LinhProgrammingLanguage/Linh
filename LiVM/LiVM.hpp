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

    private:
        std::vector<std::variant<int64_t, double, std::string, bool>> stack;
        std::unordered_map<int, std::variant<int64_t, double, std::string, bool>> variables;
        size_t ip = 0; // instruction pointer

        void push(const std::variant<int64_t, double, std::string, bool> &val);
        std::variant<int64_t, double, std::string, bool> pop();
        std::variant<int64_t, double, std::string, bool> peek();
    };
}
