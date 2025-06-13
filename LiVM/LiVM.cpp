#include "LiVM.hpp"

namespace Linh
{
    LiVM::LiVM() {}

    void LiVM::push(const std::variant<int64_t, double, std::string, bool> &val)
    {
        stack.push_back(val);
    }

    std::variant<int64_t, double, std::string, bool> LiVM::pop()
    {
        if (stack.empty())
            throw std::runtime_error("VM stack underflow");
        auto val = stack.back();
        stack.pop_back();
        return val;
    }

    std::variant<int64_t, double, std::string, bool> LiVM::peek()
    {
        if (stack.empty())
            throw std::runtime_error("VM stack empty");
        return stack.back();
    }

    void LiVM::run(const BytecodeChunk &chunk)
    {
        ip = 0;
        while (ip < chunk.size())
        {
            const auto &instr = chunk[ip];
            switch (instr.opcode)
            {
            case OpCode::PUSH_INT:
                push(std::get<int64_t>(instr.operand));
                break;
            case OpCode::PUSH_FLOAT:
                push(std::get<double>(instr.operand));
                break;
            case OpCode::PUSH_STR:
                push(std::get<std::string>(instr.operand));
                break;
            case OpCode::PUSH_BOOL:
                push(std::get<bool>(instr.operand));
                break;
            case OpCode::POP:
                pop();
                break;
            case OpCode::SWAP:
                if (stack.size() < 2)
                    throw std::runtime_error("VM stack underflow for SWAP");
                std::swap(stack[stack.size() - 1], stack[stack.size() - 2]);
                break;
            case OpCode::ADD:
            case OpCode::SUB:
            case OpCode::MUL:
            case OpCode::DIV:
            case OpCode::MOD:
            {
                auto b = pop();
                auto a = pop();
                // Only support int64_t and double for now
                if (std::holds_alternative<int64_t>(a) && std::holds_alternative<int64_t>(b))
                {
                    int64_t av = std::get<int64_t>(a);
                    int64_t bv = std::get<int64_t>(b);
                    switch (instr.opcode)
                    {
                    case OpCode::ADD:
                        push(av + bv);
                        break;
                    case OpCode::SUB:
                        push(av - bv);
                        break;
                    case OpCode::MUL:
                        push(av * bv);
                        break;
                    case OpCode::DIV:
                        push(av / bv);
                        break;
                    case OpCode::MOD:
                        push(av % bv);
                        break;
                    default:
                        break;
                    }
                }
                else if ((std::holds_alternative<int64_t>(a) || std::holds_alternative<double>(a)) &&
                         (std::holds_alternative<int64_t>(b) || std::holds_alternative<double>(b)))
                {
                    double av = std::holds_alternative<int64_t>(a) ? static_cast<double>(std::get<int64_t>(a)) : std::get<double>(a);
                    double bv = std::holds_alternative<int64_t>(b) ? static_cast<double>(std::get<int64_t>(b)) : std::get<double>(b);
                    switch (instr.opcode)
                    {
                    case OpCode::ADD:
                        push(av + bv);
                        break;
                    case OpCode::SUB:
                        push(av - bv);
                        break;
                    case OpCode::MUL:
                        push(av * bv);
                        break;
                    case OpCode::DIV:
                        push(av / bv);
                        break;
                    case OpCode::MOD:
                        push(static_cast<int64_t>(av) % static_cast<int64_t>(bv));
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    throw std::runtime_error("VM: ADD/SUB/MUL/DIV/MOD only supports int/float");
                }
                break;
            }
            case OpCode::AND:
            case OpCode::OR:
            {
                auto b = pop();
                auto a = pop();
                if (std::holds_alternative<bool>(a) && std::holds_alternative<bool>(b))
                {
                    bool av = std::get<bool>(a);
                    bool bv = std::get<bool>(b);
                    if (instr.opcode == OpCode::AND)
                        push(av && bv);
                    else
                        push(av || bv);
                }
                else
                {
                    throw std::runtime_error("VM: AND/OR only supports bool");
                }
                break;
            }
            case OpCode::NOT:
            {
                auto a = pop();
                if (std::holds_alternative<bool>(a))
                    push(!std::get<bool>(a));
                else
                    throw std::runtime_error("VM: NOT only supports bool");
                break;
            }
            case OpCode::EQ:
            case OpCode::NEQ:
            case OpCode::LT:
            case OpCode::GT:
            case OpCode::LTE:
            case OpCode::GTE:
            {
                auto b = pop();
                auto a = pop();
                // Only support int64_t and double for now
                double av = std::holds_alternative<int64_t>(a) ? static_cast<double>(std::get<int64_t>(a)) : std::get<double>(a);
                double bv = std::holds_alternative<int64_t>(b) ? static_cast<double>(std::get<int64_t>(b)) : std::get<double>(b);
                bool result = false;
                switch (instr.opcode)
                {
                case OpCode::EQ:
                    result = (av == bv);
                    break;
                case OpCode::NEQ:
                    result = (av != bv);
                    break;
                case OpCode::LT:
                    result = (av < bv);
                    break;
                case OpCode::GT:
                    result = (av > bv);
                    break;
                case OpCode::LTE:
                    result = (av <= bv);
                    break;
                case OpCode::GTE:
                    result = (av >= bv);
                    break;
                default:
                    break;
                }
                push(result);
                break;
            }
            case OpCode::LOAD_VAR:
            {
                int idx = std::get<int64_t>(instr.operand);
                if (variables.count(idx))
                    push(variables[idx]);
                else
                    throw std::runtime_error("VM: LOAD_VAR unknown variable index");
                break;
            }
            case OpCode::STORE_VAR:
            {
                int idx = std::get<int64_t>(instr.operand);
                variables[idx] = pop();
                break;
            }
            case OpCode::PRINT:
            {
                auto val = pop();
                std::cout << std::boolalpha;
                std::visit([](auto &&arg)
                           { std::cout << arg << std::endl; }, val);
                break;
            }
            case OpCode::HALT:
                return;
            default:
                // NOP or not implemented
                break;
            }
            ++ip;
        }
    }
}
