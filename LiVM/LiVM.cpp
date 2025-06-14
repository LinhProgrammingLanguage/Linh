#include "LiVM.hpp"
#include "iostream/iostream.hpp"
#include "Loop.hpp"
#ifdef _WIN32
#include <windows.h>
#else
#include <locale>
#endif

void force_console_utf8()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#else
    std::locale::global(std::locale(""));
    std::cout.imbue(std::locale());
#endif
}

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
        {
            std::cerr << "VM stack underflow" << std::endl;
            return int64_t(0);
        }
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
        force_console_utf8();
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
                {
                    std::cerr << "VM stack underflow for SWAP" << std::endl;
                    break;
                }
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
                    std::cerr << "VM: ADD/SUB/MUL/DIV/MOD only supports int/float" << std::endl;
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
                    std::cerr << "VM: AND/OR only supports bool" << std::endl;
                }
                break;
            }
            case OpCode::NOT:
            {
                auto a = pop();
                if (std::holds_alternative<bool>(a))
                    push(!std::get<bool>(a));
                else
                    std::cerr << "VM: NOT only supports bool" << std::endl;
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
                {
                    std::cerr << "VM: LOAD_VAR unknown variable index " << idx << std::endl;
                    push(int64_t(0));
                }
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
                LinhIO::linh_print(val);
                break;
            }
            case OpCode::INPUT:
            {
                auto prompt = pop();
                std::string prompt_str;
                if (std::holds_alternative<std::string>(prompt))
                    prompt_str = std::get<std::string>(prompt);
                else
                    prompt_str = "";
                auto input_val = LinhIO::linh_input(prompt_str);
                push(input_val);
                break;
            }
            case OpCode::TYPEOF:
            {
                auto val = pop();
                if (std::holds_alternative<int64_t>(val))
                    std::cout << "int" << std::endl;
                else if (std::holds_alternative<double>(val))
                    std::cout << "float" << std::endl;
                else if (std::holds_alternative<std::string>(val))
                    std::cout << "str" << std::endl;
                else if (std::holds_alternative<bool>(val))
                    std::cout << "bool" << std::endl;
                else
                    std::cout << "unknown" << std::endl;
                break;
            }
            case OpCode::HALT:
                return;
            case OpCode::JMP:
            case OpCode::JMP_IF_FALSE:
            case OpCode::JMP_IF_TRUE:
                handle_loop_opcode(*this, instr, chunk, ip);
                continue;
            default:
                // NOP or not implemented
                break;
            }
            ++ip;
        }
    }

} // namespace Linh
