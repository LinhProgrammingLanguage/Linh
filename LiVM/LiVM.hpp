#pragma once
#include "../LinhC/Bytecode/Bytecode.hpp"
#include "Value/Value.hpp"
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <chrono>
#include <unordered_set>
#include <functional>

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

        // Optimization methods
        void enable_instruction_caching(bool enable = true) { instruction_caching_enabled = enable; }
        void enable_stack_optimization(bool enable = true) { stack_optimization_enabled = enable; }
        void enable_hot_path_optimization(bool enable = true) { hot_path_optimization_enabled = enable; }
        
        // Performance monitoring
        size_t get_execution_time_ms() const { return execution_time_ms; }
        size_t get_instruction_count() const { return instruction_count; }
        size_t get_cache_hits() const { return cache_hits; }
        size_t get_cache_misses() const { return cache_misses; }
        double get_instructions_per_ms() const { 
            return execution_time_ms > 0 ? static_cast<double>(instruction_count) / execution_time_ms : 0.0; 
        }

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

        // Getter/setter cho biến toàn cục REPL
        const std::unordered_map<int, Value> &get_global_variables() const { return variables; }
        void set_global_variables(const std::unordered_map<int, Value> &vars) { variables = vars; }

    private:
        std::vector<Value> stack;
        std::unordered_map<int, Value> variables;
        size_t ip = 0; // instruction pointer
        
        // Optimization flags
        bool instruction_caching_enabled = false; // Tắt cache để đảm bảo các lệnh có side-effect hoạt động đúng
        bool stack_optimization_enabled = true;
        bool hot_path_optimization_enabled = true;
        
        // Performance tracking
        size_t execution_time_ms = 0;
        size_t instruction_count = 0;
        size_t cache_hits = 0;
        size_t cache_misses = 0;
        
        // Instruction caching
        std::unordered_map<size_t, std::function<void()>> instruction_cache;
        std::unordered_set<size_t> hot_paths;
        
        // Stack optimization
        static constexpr size_t STACK_RESERVE_SIZE = 1024;
        static constexpr size_t STACK_SHRINK_THRESHOLD = 512;

        void push(const Value &val);
        Value pop();
        Value peek();
        
        // Optimization helpers
        void cache_instruction(size_t ip, const Instruction& instr);
        void optimize_stack();
        void track_hot_path(size_t ip);
        void execute_cached_instruction(size_t ip);
    };
}