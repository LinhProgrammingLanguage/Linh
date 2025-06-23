#include "LiVM.hpp"
#include "iostream/iostream.hpp"
#include "Loop.hpp"
#include <cmath>
#include "Math/Math.hpp" // Thêm dòng này

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

    void LiVM::push(const std::variant<std::monostate, int64_t, double, std::string, bool> &val)
    {
        stack.push_back(val);
    }

    std::variant<std::monostate, int64_t, double, std::string, bool> LiVM::pop()
    {
        if (stack.empty())
        {
            std::cerr << "[Line 0 , Col 0] RuntimeError : VM stack underflow" << std::endl;
            return std::monostate{};
        }
        auto val = stack.back();
        stack.pop_back();
        return val;
    }

    std::variant<std::monostate, int64_t, double, std::string, bool> LiVM::peek()
    {
        if (stack.empty())
            throw std::runtime_error("VM stack empty");
        return stack.back();
    }

    // Add this helper function at the top (after includes)
    static const char *opcode_name(Linh::OpCode opcode)
    {
        using Linh::OpCode;
        switch (opcode)
        {
        case OpCode::NOP:
            return "NOP";
        case OpCode::PUSH_INT:
            return "PUSH_INT";
        case OpCode::PUSH_FLOAT:
            return "PUSH_FLOAT";
        case OpCode::PUSH_STR:
            return "PUSH_STR";
        case OpCode::PUSH_BOOL:
            return "PUSH_BOOL";
        case OpCode::POP:
            return "POP";
        case OpCode::SWAP:
            return "SWAP";
        case OpCode::DUP:
            return "DUP";
        case OpCode::ADD:
            return "ADD";
        case OpCode::SUB:
            return "SUB";
        case OpCode::MUL:
            return "MUL";
        case OpCode::DIV:
            return "DIV";
        case OpCode::MOD:
            return "MOD";
        case OpCode::HASH:
            return "HASH";
        case OpCode::AMP:
            return "AMP";
        case OpCode::PIPE:
            return "PIPE";
        case OpCode::CARET:
            return "CARET";
        case OpCode::LT_LT:
            return "LT_LT";
        case OpCode::GT_GT:
            return "GT_GT";
        case OpCode::AND:
            return "AND";
        case OpCode::OR:
            return "OR";
        case OpCode::NOT:
            return "NOT";
        case OpCode::EQ:
            return "EQ";
        case OpCode::NEQ:
            return "NEQ";
        case OpCode::LT:
            return "LT";
        case OpCode::GT:
            return "GT";
        case OpCode::LTE:
            return "LTE";
        case OpCode::GTE:
            return "GTE";
        case OpCode::LOAD_VAR:
            return "LOAD_VAR";
        case OpCode::STORE_VAR:
            return "STORE_VAR";
        case OpCode::JMP:
            return "JMP";
        case OpCode::JMP_IF_FALSE:
            return "JMP_IF_FALSE";
        case OpCode::JMP_IF_TRUE:
            return "JMP_IF_TRUE";
        case OpCode::CALL:
            return "CALL";
        case OpCode::RET:
            return "RET";
        case OpCode::PRINT:
            return "PRINT";
        case OpCode::INPUT:
            return "INPUT";
        case OpCode::TYPEOF:
            return "TYPEOF";
        case OpCode::HALT:
            return "HALT";
        case OpCode::TRY:
            return "TRY";
        case OpCode::END_TRY:
            return "END_TRY";
        default:
            return "UNKNOWN";
        }
    }

    void LiVM::run(const BytecodeChunk &chunk)
    {
        ip = 0;
        std::vector<TryFrame> try_stack;

        // --- Print full instruction list for debugging ---
        std::cerr << "=== VM Bytecode Dump ===" << std::endl;
        for (size_t i = 0; i < chunk.size(); ++i)
        {
            const auto &instr = chunk[i];
            std::cerr << "[" << i << "] OpCode: " << static_cast<int>(instr.opcode)
                      << " (" << opcode_name(instr.opcode) << "), Operand: ";
            if (std::holds_alternative<int64_t>(instr.operand))
                std::cerr << std::get<int64_t>(instr.operand);
            else if (std::holds_alternative<double>(instr.operand))
                std::cerr << std::get<double>(instr.operand);
            else if (std::holds_alternative<std::string>(instr.operand))
                std::cerr << std::get<std::string>(instr.operand);
            else if (std::holds_alternative<bool>(instr.operand))
                std::cerr << (std::get<bool>(instr.operand) ? "true" : "false");
            else
                std::cerr << "(tuple/other)";
            std::cerr << std::endl;
        }
        std::cerr << "=== End Bytecode Dump ===" << std::endl;

        while (ip < chunk.size())
        {
            const auto &instr = chunk[ip];
            try
            {
                // Debug: In thông tin opcode, operand, stack size
                std::cerr << "[DEBUG][ip=" << ip << "] OpCode: " << static_cast<int>(instr.opcode)
                          << " (" << opcode_name(instr.opcode) << ")"
                          << ", Operand index: ";
                if (std::holds_alternative<int64_t>(instr.operand))
                    std::cerr << std::get<int64_t>(instr.operand);
                else if (std::holds_alternative<double>(instr.operand))
                    std::cerr << std::get<double>(instr.operand);
                else if (std::holds_alternative<std::string>(instr.operand))
                    std::cerr << std::get<std::string>(instr.operand);
                else if (std::holds_alternative<bool>(instr.operand))
                    std::cerr << (std::get<bool>(instr.operand) ? "true" : "false");
                else
                    std::cerr << "(tuple/other)";
                std::cerr << ", Stack size: " << stack.size() << std::endl;

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
                case OpCode::HASH:
                case OpCode::AMP:
                case OpCode::PIPE:
                case OpCode::CARET:
                case OpCode::LT_LT:
                case OpCode::GT_GT:
                    Linh::math_binary_op(*this, instr);
                    break;
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
                    else if (std::holds_alternative<int64_t>(a))
                        push(~std::get<int64_t>(a)); // bitwise NOT
                    else
                        std::cerr << "VM: NOT only supports bool or int" << std::endl;
                    break;
                }
                case OpCode::EQ:
                case OpCode::NEQ:
                case OpCode::LT:
                case OpCode::GT:
                case OpCode::LTE:
                case OpCode::GTE:
                {
                    // Debug: In stack trước khi pop
                    std::cerr << "[DEBUG] Stack before pop (size=" << stack.size() << "): ";
                    for (const auto &v : stack)
                    {
                        if (std::holds_alternative<int64_t>(v))
                            std::cerr << std::get<int64_t>(v) << " ";
                        else if (std::holds_alternative<double>(v))
                            std::cerr << std::get<double>(v) << " ";
                        else if (std::holds_alternative<std::string>(v))
                            std::cerr << "\"" << std::get<std::string>(v) << "\" ";
                        else if (std::holds_alternative<bool>(v))
                            std::cerr << (std::get<bool>(v) ? "true" : "false") << " ";
                        else
                            std::cerr << "(?) ";
                    }
                    std::cerr << std::endl;
                    auto b = pop();
                    auto a = pop();
                    // Debug: In giá trị a, b
                    std::cerr << "[DEBUG] a=";
                    if (std::holds_alternative<int64_t>(a))
                        std::cerr << std::get<int64_t>(a);
                    else if (std::holds_alternative<double>(a))
                        std::cerr << std::get<double>(a);
                    else if (std::holds_alternative<std::string>(a))
                        std::cerr << "\"" << std::get<std::string>(a) << "\"";
                    else if (std::holds_alternative<bool>(a))
                        std::cerr << (std::get<bool>(a) ? "true" : "false");
                    else
                        std::cerr << "(?)";
                    std::cerr << ", b=";
                    if (std::holds_alternative<int64_t>(b))
                        std::cerr << std::get<int64_t>(b);
                    else if (std::holds_alternative<double>(b))
                        std::cerr << std::get<double>(b);
                    else if (std::holds_alternative<std::string>(b))
                        std::cerr << "\"" << std::get<std::string>(b) << "\"";
                    else if (std::holds_alternative<bool>(b))
                        std::cerr << (std::get<bool>(b) ? "true" : "false");
                    else
                        std::cerr << "(?)";
                    std::cerr << std::endl;

                    bool result = false;
                    // So sánh chuỗi nếu một trong hai là string
                    if (std::holds_alternative<std::string>(a) || std::holds_alternative<std::string>(b))
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
                            if ((instr.opcode == OpCode::DIV || instr.opcode == OpCode::MOD) && bv == 0)
                            {
                                std::string err_msg = "Division by zero (int)";
                                if (!try_stack.empty())
                                {
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
                        case OpCode::HASH:
                            if (bv == 0)
                            {
                                std::string err_msg = "Floor division by zero (int)";
                                if (!try_stack.empty())
                                {
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
                            // Python-like floor division for int
                            if ((av < 0) != (bv < 0) && av % bv != 0)
                                push((av / bv) - 1);
                            else
                                push(av / bv);
                            break;
                        case OpCode::AMP:
                            push(av & bv);
                            break;
                        case OpCode::PIPE:
                            push(av | bv);
                            break;
                        case OpCode::CARET:
                            push(av ^ bv);
                            break;
                        case OpCode::LT_LT:
                            push(av << bv);
                            break;
                        case OpCode::GT_GT:
                            push(av >> bv);
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
                            if (bv == 0.0)
                            {
                                std::cerr << "[Line " << instr.line << ", Col " << instr.col << "] VM: Division by zero (float)" << std::endl;
                                return;
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
                                return;
                            }
                            else
                            {
                                push(std::fmod(av, bv));
                            }
                            break;
                        case OpCode::HASH:
                            if (bv == 0.0)
                            {
                                std::cerr << "[Line " << instr.line << ", Col " << instr.col << "] VM: Floor division by zero (float)" << std::endl;
                                return;
                            }
                            else
                            {
                                push(std::floor(av / bv));
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
                case OpCode::LOAD_VAR:
                {
                    int idx = std::get<int64_t>(instr.operand);
                    std::cerr << "[DEBUG] LOAD_VAR idx=" << idx << ", variables.size()=" << variables.size() << std::endl;
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
                    if (stack.empty())
                    {
                        stack.push_back(std::monostate{});
                    }
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
                    // --- Thêm hỗ trợ hàm pow ---
                    if (fname == "pow")
                    {
                        auto b = pop();
                        auto a = pop();
                        double av = 0, bv = 0;
                        if (std::holds_alternative<int64_t>(a))
                            av = static_cast<double>(std::get<int64_t>(a));
                        else if (std::holds_alternative<double>(a))
                            av = std::get<double>(a);
                        else
                            av = 0;
                        if (std::holds_alternative<int64_t>(b))
                            bv = static_cast<double>(std::get<int64_t>(b));
                        else if (std::holds_alternative<double>(b))
                            bv = std::get<double>(b);
                        else
                            bv = 0;
                        push(std::pow(av, bv));
                        break;
                    }
                    if (!functions.count(fname))
                    {
                        std::cerr << "VM: Unknown function '" << fname << "'\n";
                        return;
                    }
                    const auto &fn = functions[fname];
                    // Pop arguments in reverse order
                    std::vector<std::variant<std::monostate, int64_t, double, std::string, bool>> args;
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
                        case OpCode::HASH:  // # (floor division)
                        case OpCode::AMP:   // & (bitwise and)
                        case OpCode::PIPE:  // | (bitwise or)
                        case OpCode::CARET: // ^ (bitwise xor)
                        case OpCode::LT_LT: // << (bitwise shift left)
                        case OpCode::GT_GT: // >> (bitwise shift right)
                        {
                            // Debug: In stack trước khi pop
                            std::cerr << "[DEBUG] Stack before pop (size=" << stack.size() << "): ";
                            for (const auto &v : stack)
                            {
                                if (std::holds_alternative<int64_t>(v))
                                    std::cerr << std::get<int64_t>(v) << " ";
                                else if (std::holds_alternative<double>(v))
                                    std::cerr << std::get<double>(v) << " ";
                                else if (std::holds_alternative<std::string>(v))
                                    std::cerr << "\"" << std::get<std::string>(v) << "\" ";
                                else if (std::holds_alternative<bool>(v))
                                    std::cerr << (std::get<bool>(v) ? "true" : "false") << " ";
                                else
                                    std::cerr << "(?) ";
                            }
                            std::cerr << std::endl;
                            auto b = pop();
                            auto a = pop();
                            // Debug: In giá trị a, b
                            std::cerr << "[DEBUG] a=";
                            if (std::holds_alternative<int64_t>(a))
                                std::cerr << std::get<int64_t>(a);
                            else if (std::holds_alternative<double>(a))
                                std::cerr << std::get<double>(a);
                            else if (std::holds_alternative<std::string>(a))
                                std::cerr << "\"" << std::get<std::string>(a) << "\"";
                            else if (std::holds_alternative<bool>(a))
                                std::cerr << (std::get<bool>(a) ? "true" : "false");
                            else
                                std::cerr << "(?)";
                            std::cerr << ", b=";
                            if (std::holds_alternative<int64_t>(b))
                                std::cerr << std::get<int64_t>(b);
                            else if (std::holds_alternative<double>(b))
                                std::cerr << std::get<double>(b);
                            else if (std::holds_alternative<std::string>(b))
                                std::cerr << "\"" << std::get<std::string>(b) << "\"";
                            else if (std::holds_alternative<bool>(b))
                                std::cerr << (std::get<bool>(b) ? "true" : "false");
                            else
                                std::cerr << "(?)";
                            std::cerr << std::endl;
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
                                    if ((finstr.opcode == OpCode::DIV || finstr.opcode == OpCode::MOD) && bv == 0)
                                    {
                                        std::string err_msg = "Division by zero (int)";
                                        if (!try_stack.empty())
                                        {
                                            variables[2] = err_msg;
                                            ip = try_stack.back().catch_ip;
                                            goto function_returned;
                                        }
                                        else
                                        {
                                            std::cerr << "[Line " << instr.line << ", Col " << instr.col << "] VM: " << err_msg << std::endl;
                                            return;
                                        }
                                    }
                                    if (finstr.opcode == OpCode::DIV)
                                        push(av / bv);
                                    else
                                        push(av % bv);
                                    break;
                                case OpCode::HASH:
                                    if (bv == 0)
                                    {
                                        std::string err_msg = "Floor division by zero (int)";
                                        if (!try_stack.empty())
                                        {
                                            variables[2] = err_msg;
                                            ip = try_stack.back().catch_ip;
                                            goto function_returned;
                                        }
                                        else
                                        {
                                            std::cerr << "[Line " << instr.line << ", Col " << instr.col << "] VM: " << err_msg << std::endl;
                                            return;
                                        }
                                    }
                                    // Python-like floor division for int
                                    if ((av < 0) != (bv < 0) && av % bv != 0)
                                        push((av / bv) - 1);
                                    else
                                        push(av / bv);
                                    break;
                                case OpCode::AMP:
                                    push(av & bv);
                                    break;
                                case OpCode::PIPE:
                                    push(av | bv);
                                    break;
                                case OpCode::CARET:
                                    push(av ^ bv);
                                    break;
                                case OpCode::LT_LT:
                                    push(av << bv);
                                    break;
                                case OpCode::GT_GT:
                                    push(av >> bv);
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
                                        return;
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
                                        return;
                                    }
                                    else
                                    {
                                        push(std::fmod(av, bv));
                                    }
                                    break;
                                case OpCode::HASH:
                                    if (bv == 0.0)
                                    {
                                        std::cerr << "[Line " << instr.line << ", Col " << instr.col << "] VM: Floor division by zero (float)" << std::endl;
                                        return;
                                    }
                                    else
                                    {
                                        push(std::floor(av / bv));
                                    }
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
                            else if (std::holds_alternative<int64_t>(a))
                                push(~std::get<int64_t>(a)); // bitwise NOT
                            else
                                std::cerr << "VM: NOT only supports bool or int" << std::endl;
                            break;
                        }
                        case OpCode::EQ:
                        case OpCode::NEQ:
                        case OpCode::LT:
                        case OpCode::GT:
                        case OpCode::LTE:
                        case OpCode::GTE:
                        {
                            // Debug: In stack trước khi pop
                            std::cerr << "[DEBUG] Stack before pop (size=" << stack.size() << "): ";
                            for (const auto &v : stack)
                            {
                                if (std::holds_alternative<int64_t>(v))
                                    std::cerr << std::get<int64_t>(v) << " ";
                                else if (std::holds_alternative<double>(v))
                                    std::cerr << std::get<double>(v) << " ";
                                else if (std::holds_alternative<std::string>(v))
                                    std::cerr << "\"" << std::get<std::string>(v) << "\" ";
                                else if (std::holds_alternative<bool>(v))
                                    std::cerr << (std::get<bool>(v) ? "true" : "false") << " ";
                                else
                                    std::cerr << "(?) ";
                            }
                            std::cerr << std::endl;
                            auto b = pop();
                            auto a = pop();
                            // Debug: In giá trị a, b
                            std::cerr << "[DEBUG] a=";
                            if (std::holds_alternative<int64_t>(a))
                                std::cerr << std::get<int64_t>(a);
                            else if (std::holds_alternative<double>(a))
                                std::cerr << std::get<double>(a);
                            else if (std::holds_alternative<std::string>(a))
                                std::cerr << "\"" << std::get<std::string>(a) << "\"";
                            else if (std::holds_alternative<bool>(a))
                                std::cerr << (std::get<bool>(a) ? "true" : "false");
                            else
                                std::cerr << "(?)";
                            std::cerr << ", b=";
                            if (std::holds_alternative<int64_t>(b))
                                std::cerr << std::get<int64_t>(b);
                            else if (std::holds_alternative<double>(b))
                                std::cerr << std::get<double>(b);
                            else if (std::holds_alternative<std::string>(b))
                                std::cerr << "\"" << std::get<std::string>(b) << "\"";
                            else if (std::holds_alternative<bool>(b))
                                std::cerr << (std::get<bool>(b) ? "true" : "false");
                            else
                                std::cerr << "(?)";
                            std::cerr << std::endl;

                            bool result = false;
                            // So sánh chuỗi nếu một trong hai là string
                            if (std::holds_alternative<std::string>(a) || std::holds_alternative<std::string>(b))
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
                                // Do NOT push(sa + sb) for comparison ops!
                                // Instead, compare as strings:
                                switch (instr.opcode)
                                {
                                case OpCode::EQ:
                                    result = (sa == sb);
                                    break;
                                case OpCode::NEQ:
                                    result = (sa != sb);
                                    break;
                                case OpCode::LT:
                                    result = (sa < sb);
                                    break;
                                case OpCode::GT:
                                    result = (sa > sb);
                                    break;
                                case OpCode::LTE:
                                    result = (sa <= sb);
                                    break;
                                case OpCode::GTE:
                                    result = (sa >= sb);
                                    break;
                                default:
                                    break;
                                }
                                push(result);
                                break;
                            }
                            // So sánh bool nếu cả hai là bool
                            if (std::holds_alternative<bool>(a) && std::holds_alternative<bool>(b))
                            {
                                bool av = std::get<bool>(a);
                                bool bv = std::get<bool>(b);
                                switch (instr.opcode)
                                {
                                case OpCode::EQ:
                                    result = (av == bv);
                                    break;
                                case OpCode::NEQ:
                                    result = (av != bv);
                                    break;
                                case OpCode::LT:
                                    result = (!av && bv);
                                    break;
                                case OpCode::GT:
                                    result = (av && !bv);
                                    break;
                                case OpCode::LTE:
                                    result = (!av || bv);
                                    break;
                                case OpCode::GTE:
                                    result = (av || !bv);
                                    break;
                                default:
                                    break;
                                }
                                push(result);
                                break;
                            }
                            // So sánh số nếu cả hai là số
                            if ((std::holds_alternative<int64_t>(a) || std::holds_alternative<double>(a)) &&
                                (std::holds_alternative<int64_t>(b) || std::holds_alternative<double>(b)))
                            {
                                double av = std::holds_alternative<int64_t>(a) ? static_cast<double>(std::get<int64_t>(a)) : std::get<double>(a);
                                double bv = std::holds_alternative<int64_t>(b) ? static_cast<double>(std::get<int64_t>(b)) : std::get<double>(b);
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
                            // Nếu kiểu không khớp, chuyển về chuỗi rồi so sánh
                            std::string sa, sb;
                            // a
                            if (std::holds_alternative<int64_t>(a))
                                sa = std::to_string(std::get<int64_t>(a));
                            else if (std::holds_alternative<double>(a))
                                sa = std::to_string(std::get<double>(a));
                            else if (std::holds_alternative<bool>(a))
                                sa = std::get<bool>(a) ? "true" : "false";
                            else if (std::holds_alternative<std::string>(a))
                                sa = std::get<std::string>(a);
                            else
                                sa = "";
                            // b
                            if (std::holds_alternative<int64_t>(b))
                                sb = std::to_string(std::get<int64_t>(b));
                            else if (std::holds_alternative<double>(b))
                                sb = std::to_string(std::get<double>(b));
                            else if (std::holds_alternative<bool>(b))
                                sb = std::get<bool>(b) ? "true" : "false";
                            else if (std::holds_alternative<std::string>(b))
                                sb = std::get<std::string>(b);
                            else
                                sb = "";
                            switch (instr.opcode)
                            {
                            case OpCode::EQ:
                                result = (sa == sb);
                                break;
                            case OpCode::NEQ:
                                result = (sa != sb);
                                break;
                            case OpCode::LT:
                                result = (sa < sb);
                                break;
                            case OpCode::GT:
                                result = (sa > sb);
                                break;
                            case OpCode::LTE:
                                result = (sa <= sb);
                                break;
                            case OpCode::GTE:
                                result = (sa >= sb);
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
                            std::cerr << "[DEBUG] LOAD_VAR idx=" << idx << ", variables.size()=" << variables.size() << std::endl;
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
                            if (stack.empty())
                            {
                                stack.push_back(std::monostate{});
                            }
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
                            // --- Fix: Always push a value to the stack before returning ---
                            if (stack.empty())
                            {
                                stack.push_back(std::monostate{});
                            }
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
                    // --- Fix: Always push a value to the stack before returning from global code (should not happen, but for safety) ---
                    if (stack.empty())
                    {
                        stack.push_back(std::monostate{});
                    }
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
                case OpCode::DUP:
                    std::cerr << "[DEBUG] DUP stack size before: " << stack.size() << std::endl;
                    if (stack.empty())
                    {
                        std::cerr << "VM stack underflow for DUP" << std::endl;
                        break;
                    }
                    stack.push_back(stack.back());
                    std::cerr << "[DEBUG] DUP stack size after: " << stack.size() << std::endl;
                    break;
                default:
                    // NOP or not implemented
                    break;
                }
            }
            catch (const std::exception &ex)
            {
                std::cerr << "[DEBUG][EXCEPTION] " << ex.what() << " at ip=" << ip << std::endl;
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