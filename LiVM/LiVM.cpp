#include "LiVM.hpp"
#include "iostream/iostream.hpp"
#include "Loop.hpp"
#ifdef _WIN32
#include <windows.h>
#else
#include <locale>
#endif
#include <cmath>

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

struct TryFrame
{
    size_t catch_ip;
    size_t finally_ip;
    size_t end_ip;
    std::string error_var;
    TryFrame(size_t c, size_t f, size_t e, std::string err = "error")
        : catch_ip(c), finally_ip(f), end_ip(e), error_var(std::move(err)) {}
};

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
        std::vector<TryFrame> try_stack;

        while (ip < chunk.size())
        {
            const auto &instr = chunk[ip];
            try
            {
                switch (instr.opcode)
                {
                case OpCode::PUSH_INT:
                    // If you want to support int128, check here
                    // For now, always push int64_t
                    push(std::get<int64_t>(instr.operand));
                    break;
                case OpCode::PUSH_FLOAT:
                    // If you want to support float128, check here
                    // For now, always push double
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
                    // --- HỖ TRỢ NỐI CHUỖI ---
                    if (instr.opcode == OpCode::ADD &&
                        (std::holds_alternative<std::string>(a) || std::holds_alternative<std::string>(b)))
                    {
                        std::string sa, sb;
                        if (std::holds_alternative<std::string>(a))
                            sa = std::get<std::string>(a);
                        else if (std::holds_alternative<int64_t>(a))
                            sa = std::to_string(std::get<int64_t>(a));
                        else if (std::holds_alternative<double>(a))
                            sa = std::to_string(std::get<double>(a));
                        else if (std::holds_alternative<bool>(a))
                            sa = std::get<bool>(a) ? "true" : "false";
                        if (std::holds_alternative<std::string>(b))
                            sb = std::get<std::string>(b);
                        else if (std::holds_alternative<int64_t>(b))
                            sb = std::to_string(std::get<int64_t>(b));
                        else if (std::holds_alternative<double>(b))
                            sb = std::to_string(std::get<double>(b));
                        else if (std::holds_alternative<bool>(b))
                            sb = std::get<bool>(b) ? "true" : "false";
                        push(sa + sb);
                        break;
                    }
                    // --- KẾT THÚC HỖ TRỢ NỐI CHUỖI ---
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
                        case OpCode::MOD:
                        {
                            auto b = pop();
                            auto a = pop();
                            if (std::holds_alternative<int64_t>(a) && std::holds_alternative<int64_t>(b))
                            {
                                int64_t av = std::get<int64_t>(a);
                                int64_t bv = std::get<int64_t>(b);
                                if ((instr.opcode == OpCode::DIV || instr.opcode == OpCode::MOD) && bv == 0)
                                {
                                    std::string err_msg = "Division by zero (int)";
                                    if (!try_stack.empty())
                                    {
                                        // Không in ra std::cerr ở đây!
                                        variables[2] = err_msg;
                                        ip = try_stack.back().catch_ip;
                                        continue;
                                    }
                                    else
                                    {
                                        std::cerr << "[Line " << instr.line << ", Col " << instr.col << "] VM: " << err_msg << std::endl;
                                        return;
                                    }
                                }
                                if (instr.opcode == OpCode::DIV)
                                    push(av / bv);
                                else
                                    push(av % bv);
                                break;
                            }
                        }
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
                            if (bv == 0.0)
                            {
                                std::cerr << "[Line " << instr.line << ", Col " << instr.col << "] VM: Division by zero (float)" << std::endl;
                                return; // Dừng thực thi VM
                            }
                            else
                            {
                                push(av / bv);
                            }
                            break;
                        case OpCode::MOD:
                            if (bv == 0.0)
                            {
                                std::cerr << "[Line " << instr.line << ", Col " << instr.col << "] VM: Modulo by zero (float)" << std::endl;
                                return; // Dừng thực thi VM
                            }
                            else
                            {
                                push(std::fmod(av, bv));
                            }
                            break;
                        default:
                            break;
                        }
                    }
                    else
                    {
                        std::cerr << "[Line " << instr.line << ", Col " << instr.col << "] VM: Invalid operand types for arithmetic" << std::endl;
                        return; // Dừng thực thi VM nếu lỗi kiểu
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
                        // Nếu tên biến là error.message và error tồn tại, trả về error
                        if (idx == 3 && variables.count(2))
                        {
                            push(variables[2]);
                        }
                        else
                        {
                            std::cerr << "VM: LOAD_VAR unknown variable index " << idx << std::endl;
                            push(int64_t(0));
                        }
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
                case OpCode::CALL:
                {
                    // Get function name
                    std::string fname = std::get<std::string>(instr.operand);
                    if (!functions.count(fname))
                    {
                        std::cerr << "VM: Unknown function '" << fname << "'\n";
                        return;
                    }
                    const auto &fn = functions[fname];
                    // Pop arguments in reverse order
                    std::vector<std::variant<int64_t, double, std::string, bool>> args;
                    for (size_t i = 0; i < fn.param_names.size(); ++i)
                        args.push_back(pop());
                    std::reverse(args.begin(), args.end());
                    // Save current frame
                    CallFrame frame;
                    frame.return_ip = ip + 1;
                    frame.locals = variables;
                    call_stack.push_back(frame);
                    // Set up new variables for function
                    variables.clear();
                    for (size_t i = 0; i < fn.param_names.size(); ++i)
                        variables[i] = args[i];
                    // Run function code
                    size_t saved_ip = ip;
                    size_t fn_ip = 0;
                    while (fn_ip < fn.code.size())
                    {
                        const auto &finstr = fn.code[fn_ip];
                        switch (finstr.opcode)
                        {
                        case OpCode::PUSH_INT:
                            push(std::get<int64_t>(finstr.operand));
                            break;
                        case OpCode::PUSH_FLOAT:
                            push(std::get<double>(finstr.operand));
                            break;
                        case OpCode::PUSH_STR:
                            push(std::get<std::string>(finstr.operand));
                            break;
                        case OpCode::PUSH_BOOL:
                            push(std::get<bool>(finstr.operand));
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
                            // --- HỖ TRỢ NỐI CHUỖI ---
                            if (finstr.opcode == OpCode::ADD &&
                                (std::holds_alternative<std::string>(a) || std::holds_alternative<std::string>(b)))
                            {
                                std::string sa, sb;
                                // Chuyển a về string
                                if (std::holds_alternative<std::string>(a))
                                    sa = std::get<std::string>(a);
                                else if (std::holds_alternative<int64_t>(a))
                                    sa = std::to_string(std::get<int64_t>(a));
                                else if (std::holds_alternative<double>(a))
                                    sa = std::to_string(std::get<double>(a));
                                else if (std::holds_alternative<bool>(a))
                                    sa = std::get<bool>(a) ? "true" : "false";
                                // Chuyển b về string
                                if (std::holds_alternative<std::string>(b))
                                    sb = std::get<std::string>(b);
                                else if (std::holds_alternative<int64_t>(b))
                                    sb = std::to_string(std::get<int64_t>(b));
                                else if (std::holds_alternative<double>(b))
                                    sb = std::to_string(std::get<double>(b));
                                else if (std::holds_alternative<bool>(b))
                                    sb = std::get<bool>(b) ? "true" : "false";
                                push(sa + sb);
                                break;
                            }
                            // --- KẾT THÚC HỖ TRỢ NỐI CHUỖI ---
                            {
                                auto b = pop();
                                auto a = pop();
                                // --- Support int128/float128 if needed ---
                                if (std::holds_alternative<int64_t>(a) && std::holds_alternative<int64_t>(b))
                                {
                                    int64_t av = std::get<int64_t>(a);
                                    int64_t bv = std::get<int64_t>(b);
                                    switch (finstr.opcode)
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
                                    case OpCode::MOD:
                                    {
                                        auto b = pop();
                                        auto a = pop();
                                        if (std::holds_alternative<int64_t>(a) && std::holds_alternative<int64_t>(b))
                                        {
                                            int64_t av = std::get<int64_t>(a);
                                            int64_t bv = std::get<int64_t>(b);
                                            if ((instr.opcode == OpCode::DIV || instr.opcode == OpCode::MOD) && bv == 0)
                                            {
                                                std::string err_msg = "Division by zero (int)";
                                                if (!try_stack.empty())
                                                {
                                                    // Không in ra std::cerr ở đây!
                                                    variables[2] = err_msg;
                                                    ip = try_stack.back().catch_ip;
                                                    continue;
                                                }
                                                else
                                                {
                                                    std::cerr << "[Line " << instr.line << ", Col " << instr.col << "] VM: " << err_msg << std::endl;
                                                    return; // Dừng thực thi VM
                                                }
                                            }
                                            if (instr.opcode == OpCode::DIV)
                                                push(av / bv);
                                            else
                                                push(av % bv);
                                            break;
                                        }
                                    }
                                    default:
                                        break;
                                    }
                                }
                                else if ((std::holds_alternative<int64_t>(a) || std::holds_alternative<double>(a)) &&
                                         (std::holds_alternative<int64_t>(b) || std::holds_alternative<double>(b)))
                                {
                                    double av = std::holds_alternative<int64_t>(a) ? static_cast<double>(std::get<int64_t>(a)) : std::get<double>(a);
                                    double bv = std::holds_alternative<int64_t>(b) ? static_cast<double>(std::get<int64_t>(b)) : std::get<double>(b);
                                    switch (finstr.opcode)
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
                                        if (bv == 0.0)
                                        {
                                            std::cerr << "[Line " << instr.line << ", Col " << instr.col << "] VM: Division by zero (float)" << std::endl;
                                            return; // Dừng thực thi VM
                                        }
                                        else
                                        {
                                            push(av / bv);
                                        }
                                        break;
                                    case OpCode::MOD:
                                        if (bv == 0.0)
                                        {
                                            std::cerr << "[Line " << instr.line << ", Col " << instr.col << "] VM: Modulo by zero (float)" << std::endl;
                                            return; // Dừng thực thi VM
                                        }
                                        else
                                        {
                                            push(std::fmod(av, bv));
                                        }
                                        break;
                                    default:
                                        break;
                                    }
                                }
                                // --- Support int128/float128 using boost if needed ---
                                // else if (std::holds_alternative<int128_t>(a) && std::holds_alternative<int128_t>(b)) { ... }
                                // else if (std::holds_alternative<float128_t>(a) && std::holds_alternative<float128_t>(b)) { ... }
                                else
                                {
                                    std::cerr << "VM: ADD/SUB/MUL/DIV/MOD only supports int/float" << std::endl;
                                }
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
                                if (finstr.opcode == OpCode::AND)
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
                            switch (finstr.opcode)
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
                            int idx = std::get<int64_t>(finstr.operand);
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
                            int idx = std::get<int64_t>(finstr.operand);
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
                        case OpCode::RET:
                            // Restore previous frame
                            if (!call_stack.empty())
                            {
                                variables = call_stack.back().locals;
                                ip = call_stack.back().return_ip;
                                call_stack.pop_back();
                            }
                            else
                            {
                                ip = chunk.size(); // End program
                            }
                            goto function_returned;
                        default:
                            break;
                        }
                        ++fn_ip;
                    }
                function_returned:
                    continue;
                }
                case OpCode::RET:
                    // Should not happen in global code
                    return;
                // --- Thêm xử lý TRY-CATCH-FINALLY ---
                case OpCode::TRY:
                {
                    // operand = tuple (catch_ip, finally_ip, end_ip, error_var)
                    const auto &info = std::get<std::tuple<int64_t, int64_t, int64_t, std::string>>(instr.operand);
                    try_stack.emplace_back(
                        static_cast<size_t>(std::get<0>(info)),
                        static_cast<size_t>(std::get<1>(info)),
                        static_cast<size_t>(std::get<2>(info)),
                        std::get<3>(info));
                    break;
                }
                case OpCode::END_TRY:
                {
                    if (!try_stack.empty())
                        try_stack.pop_back();
                    break;
                }
                default:
                    // NOP or not implemented
                    break;
                }
            }
            catch (const std::exception &ex)
            {
                if (!try_stack.empty())
                {
                    variables[2] = std::string(ex.what());
                    ip = try_stack.back().catch_ip;
                    continue;
                }
                else
                {
                    std::cerr << "VM Exception: " << ex.what() << std::endl;
                    return;
                }
            }
            ++ip;
        }
    }

    void LiVM::type()
    {
        if (stack.empty())
        {
            std::cout << "stack_empty" << std::endl;
            return;
        }
        const auto &val = stack.back();
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
    }

} // namespace Linh