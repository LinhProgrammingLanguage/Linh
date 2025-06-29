#include "Math.hpp"
#include <cmath>
#include <iostream>
#include <variant>
#include <string>

namespace Linh
{
    void math_binary_op(LiVM &vm, const Instruction &instr)
    {
        // Debug: In stack trước khi pop
        std::cerr << "[DEBUG] Stack before pop (size=" << vm.stack.size() << "): ";
        for (const auto &v : vm.stack)
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
        auto b = vm.pop();
        auto a = vm.pop();
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
            vm.push(sa + sb);
            return;
        }
        // --- KẾT THÚC HỖ TRỢ NỐI CHUỖI ---
        // Ưu tiên xử lý uint64_t trước
        if (std::holds_alternative<uint64_t>(a) && std::holds_alternative<uint64_t>(b))
        {
            uint64_t av = std::get<uint64_t>(a);
            uint64_t bv = std::get<uint64_t>(b);
            switch (instr.opcode)
            {
            case OpCode::ADD:
                vm.push(av + bv);
                break;
            case OpCode::SUB:
                vm.push(av - bv);
                break;
            case OpCode::MUL:
                vm.push(av * bv);
                break;
            case OpCode::DIV:
                if (bv == 0)
                {
                    std::string err_msg = "Division by zero (uint)";
                    std::cerr << err_msg << std::endl;
                    vm.push(std::monostate{});
                    break;
                }
                vm.push(av / bv);
                break;
            case OpCode::MOD:
                if (bv == 0)
                {
                    std::string err_msg = "Modulo by zero (uint)";
                    std::cerr << err_msg << std::endl;
                    vm.push(std::monostate{});
                    break;
                }
                vm.push(av % bv);
                break;
            case OpCode::HASH:
                if (bv == 0)
                {
                    std::string err_msg = "Floor division by zero (uint)";
                    std::cerr << err_msg << std::endl;
                    vm.push(std::monostate{});
                    break;
                }
                // Floor division cho uint giống chia thường
                vm.push(av / bv);
                break;
            case OpCode::AMP:
                vm.push(av & bv);
                break;
            case OpCode::PIPE:
                vm.push(av | bv);
                break;
            case OpCode::CARET:
                vm.push(av ^ bv);
                break;
            case OpCode::LT_LT:
                vm.push(av << bv);
                break;
            case OpCode::GT_GT:
                vm.push(av >> bv);
                break;
            default:
                break;
            }
        }
        else if (std::holds_alternative<int64_t>(a) && std::holds_alternative<int64_t>(b))
        {
            int64_t av = std::get<int64_t>(a);
            int64_t bv = std::get<int64_t>(b);
            switch (instr.opcode)
            {
            case OpCode::ADD:
                vm.push(av + bv);
                break;
            case OpCode::SUB:
                vm.push(av - bv);
                break;
            case OpCode::MUL:
                vm.push(av * bv);
                break;
            case OpCode::DIV:
            case OpCode::MOD:
                if ((instr.opcode == OpCode::DIV || instr.opcode == OpCode::MOD) && bv == 0)
                {
                    std::string err_msg = "Division by zero (int)";
                    std::cerr << err_msg << std::endl;
                    vm.push(std::monostate{});
                    break;
                }
                if (instr.opcode == OpCode::DIV)
                    vm.push(av / bv);
                else
                    vm.push(av % bv);
                break;
            case OpCode::HASH:
                if (bv == 0)
                {
                    std::string err_msg = "Floor division by zero (int)";
                    std::cerr << err_msg << std::endl;
                    vm.push(std::monostate{});
                    break;
                }
                // Python-like floor division for int
                if ((av < 0) != (bv < 0) && av % bv != 0)
                    vm.push((av / bv) - 1);
                else
                    vm.push(av / bv);
                break;
            case OpCode::AMP:
                vm.push(av & bv);
                break;
            case OpCode::PIPE:
                vm.push(av | bv);
                break;
            case OpCode::CARET:
                vm.push(av ^ bv);
                break;
            case OpCode::LT_LT:
                vm.push(av << bv);
                break;
            case OpCode::GT_GT:
                vm.push(av >> bv);
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
                vm.push(av + bv);
                break;
            case OpCode::SUB:
                vm.push(av - bv);
                break;
            case OpCode::MUL:
                vm.push(av * bv);
                break;
            case OpCode::DIV:
                if (bv == 0.0)
                {
                    std::cerr << "Division by zero (float)" << std::endl;
                    vm.push(std::monostate{});
                }
                else
                {
                    vm.push(av / bv);
                }
                break;
            case OpCode::MOD:
                if (bv == 0.0)
                {
                    std::cerr << "Modulo by zero (float)" << std::endl;
                    vm.push(std::monostate{});
                }
                else
                {
                    vm.push(std::fmod(av, bv));
                }
                break;
            case OpCode::HASH:
                if (bv == 0.0)
                {
                    std::cerr << "Floor division by zero (float)" << std::endl;
                    vm.push(std::monostate{});
                }
                else
                {
                    vm.push(std::floor(av / bv));
                }
                break;
            default:
                break;
            }
        }
        else if ((std::holds_alternative<uint64_t>(a) || std::holds_alternative<double>(a)) &&
                 (std::holds_alternative<uint64_t>(b) || std::holds_alternative<double>(b)))
        {
            // Nếu một bên là double thì ép sang double, còn lại đã xử lý uint64_t ở trên
            double av = std::holds_alternative<uint64_t>(a) ? static_cast<double>(std::get<uint64_t>(a)) : std::get<double>(a);
            double bv = std::holds_alternative<uint64_t>(b) ? static_cast<double>(std::get<uint64_t>(b)) : std::get<double>(b);
            switch (instr.opcode)
            {
            case OpCode::ADD:
                vm.push(av + bv);
                break;
            case OpCode::SUB:
                vm.push(av - bv);
                break;
            case OpCode::MUL:
                vm.push(av * bv);
                break;
            case OpCode::DIV:
                if (bv == 0.0)
                {
                    std::cerr << "Division by zero (float/uint)" << std::endl;
                    vm.push(std::monostate{});
                }
                else
                {
                    vm.push(av / bv);
                }
                break;
            case OpCode::MOD:
                if (bv == 0.0)
                {
                    std::cerr << "Modulo by zero (float/uint)" << std::endl;
                    vm.push(std::monostate{});
                }
                else
                {
                    vm.push(std::fmod(av, bv));
                }
                break;
            case OpCode::HASH:
                if (bv == 0.0)
                {
                    std::cerr << "Floor division by zero (float/uint)" << std::endl;
                    vm.push(std::monostate{});
                }
                else
                {
                    vm.push(std::floor(av / bv));
                }
                break;
            default:
                break;
            }
        }
        else
        {
            std::cerr << "Invalid operand types for arithmetic" << std::endl;
            vm.push(std::monostate{});
        }
    }
}
