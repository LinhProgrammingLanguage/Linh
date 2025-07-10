#include "Math.hpp"
#include "../type.hpp"
#include <cmath>
#include <iostream>
#include <variant>
#include <string>

#ifdef _DEBUG
// Helper to print a Value for debug
static void debug_print_value(const Linh::Value& v, std::ostream& os = std::cerr) {
    os << "[" << Linh::type_of(v) << "] ";
    os << Linh::to_str(v);
}
#endif

namespace Linh
{
    void math_binary_op(LiVM &vm, const Instruction &instr)
    {
        if (vm.stack.size() < 2) {
            std::cerr << "VM stack underflow for binary operation" << std::endl;
            return;
        }
        auto b = vm.stack.back();
        vm.stack.pop_back();
        auto a = vm.stack.back();
        vm.stack.pop_back();
#ifdef _DEBUG
        std::cerr << "[DEBUG] Stack before pop (size=" << vm.stack.size() << "): ";
        debug_print_value(a);
        std::cerr << ", ";
        debug_print_value(b);
        std::cerr << std::endl;
#endif
        // --- CHẶN CỘNG SAI KIỂU DỮ LIỆU ---
        if (instr.opcode == OpCode::ADD)
        {
            // Nếu một trong hai toán hạng là bool thì không cho phép cộng
            if (std::holds_alternative<bool>(a) || std::holds_alternative<bool>(b))
            {
#ifdef _DEBUG
                std::cerr << "[ERROR] Invalid operand types for '+' (bool is not allowed): ";
                if (std::holds_alternative<bool>(a))
                    std::cerr << (std::get<bool>(a) ? "true" : "false");
                else if (std::holds_alternative<std::string>(a))
                    std::cerr << '"' << std::get<std::string>(a) << '"';
                else if (std::holds_alternative<int64_t>(a))
                    std::cerr << std::get<int64_t>(a);
                else if (std::holds_alternative<uint64_t>(a))
                    std::cerr << std::get<uint64_t>(a);
                else if (std::holds_alternative<double>(a))
                    std::cerr << std::get<double>(a);
                else
                    std::cerr << "(?)";
                std::cerr << " + ";
                if (std::holds_alternative<bool>(b))
                    std::cerr << (std::get<bool>(b) ? "true" : "false");
                else if (std::holds_alternative<std::string>(b))
                    std::cerr << '"' << std::get<std::string>(b) << '"';
                else if (std::holds_alternative<int64_t>(b))
                    std::cerr << std::get<int64_t>(b);
                else if (std::holds_alternative<uint64_t>(b))
                    std::cerr << std::get<uint64_t>(b);
                else if (std::holds_alternative<double>(b))
                    std::cerr << std::get<double>(b);
                else
                    std::cerr << "(?)";
                std::cerr << std::endl;
#endif
                vm.push(std::monostate{});
                return;
            }
        }
        // --- HỖ TRỢ NỐI CHUỖI ---
        if (instr.opcode == OpCode::ADD &&
            (std::holds_alternative<std::string>(a) || std::holds_alternative<std::string>(b)))
        {
            // Nếu một trong hai toán hạng là bool thì không cho phép nối chuỗi
            if (std::holds_alternative<bool>(a) || std::holds_alternative<bool>(b))
            {
#ifdef _DEBUG
                std::cerr << "[ERROR] Invalid operand types for string concatenation: cannot concatenate string and bool." << std::endl;
#endif
                vm.push(std::monostate{});
                return;
            }
            std::string sa, sb;
            if (std::holds_alternative<std::string>(a))
                sa = std::get<std::string>(a);
            else if (std::holds_alternative<int64_t>(a))
                sa = std::to_string(std::get<int64_t>(a));
            else if (std::holds_alternative<double>(a))
                sa = std::to_string(std::get<double>(a));
            else if (std::holds_alternative<uint64_t>(a))
                sa = std::to_string(std::get<uint64_t>(a));
            if (std::holds_alternative<std::string>(b))
                sb = std::get<std::string>(b);
            else if (std::holds_alternative<int64_t>(b))
                sb = std::to_string(std::get<int64_t>(b));
            else if (std::holds_alternative<double>(b))
                sb = std::to_string(std::get<double>(b));
            else if (std::holds_alternative<uint64_t>(b))
                sb = std::to_string(std::get<uint64_t>(b));
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
#ifdef _DEBUG
                    std::cerr << err_msg << std::endl;
#endif
                    vm.push(std::monostate{});
                    break;
                }
                vm.push(av / bv);
                break;
            case OpCode::MOD:
                if (bv == 0)
                {
                    std::string err_msg = "Modulo by zero (uint)";
#ifdef _DEBUG
                    std::cerr << err_msg << std::endl;
#endif
                    vm.push(std::monostate{});
                    break;
                }
                vm.push(av % bv);
                break;
            case OpCode::HASH:
                if (bv == 0)
                {
                    std::string err_msg = "Floor division by zero (uint)";
#ifdef _DEBUG
                    std::cerr << err_msg << std::endl;
#endif
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
            // --- So sánh ---
            case OpCode::LT:
#ifdef _DEBUG
                std::cerr << "[DEBUG][uint64_t LT] a=" << av << ", b=" << bv << ", result=" << (av < bv) << std::endl;
#endif
                vm.push(av < bv);
                break;
            case OpCode::LTE:
#ifdef _DEBUG
                std::cerr << "[DEBUG][uint64_t LTE] a=" << av << ", b=" << bv << ", result=" << (av <= bv) << std::endl;
#endif
                vm.push(av <= bv);
                break;
            case OpCode::GT:
#ifdef _DEBUG
                std::cerr << "[DEBUG][uint64_t GT] a=" << av << ", b=" << bv << ", result=" << (av > bv) << std::endl;
#endif
                vm.push(av > bv);
                break;
            case OpCode::GTE:
#ifdef _DEBUG
                std::cerr << "[DEBUG][uint64_t GTE] a=" << av << ", b=" << bv << ", result=" << (av >= bv) << std::endl;
#endif
                vm.push(av >= bv);
                break;
            case OpCode::EQ:
#ifdef _DEBUG
                std::cerr << "[DEBUG][uint64_t EQ] a=" << av << ", b=" << bv << ", result=" << (av == bv) << std::endl;
#endif
                vm.push(av == bv);
                break;
            case OpCode::NEQ:
#ifdef _DEBUG
                std::cerr << "[DEBUG][uint64_t NEQ] a=" << av << ", b=" << bv << ", result=" << (av != bv) << std::endl;
#endif
                vm.push(av != bv);
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
#ifdef _DEBUG
                    std::cerr << err_msg << std::endl;
#endif
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
#ifdef _DEBUG
                    std::cerr << err_msg << std::endl;
#endif
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
            // --- So sánh ---
            case OpCode::LT:
#ifdef _DEBUG
                std::cerr << "[DEBUG][int64_t LT] a=" << av << ", b=" << bv << ", result=" << (av < bv) << std::endl;
#endif
                vm.push(av < bv);
                break;
            case OpCode::LTE:
#ifdef _DEBUG
                std::cerr << "[DEBUG][int64_t LTE] a=" << av << ", b=" << bv << ", result=" << (av <= bv) << std::endl;
#endif
                vm.push(av <= bv);
                break;
            case OpCode::GT:
#ifdef _DEBUG
                std::cerr << "[DEBUG][int64_t GT] a=" << av << ", b=" << bv << ", result=" << (av > bv) << std::endl;
#endif
                vm.push(av > bv);
                break;
            case OpCode::GTE:
#ifdef _DEBUG
                std::cerr << "[DEBUG][int64_t GTE] a=" << av << ", b=" << bv << ", result=" << (av >= bv) << std::endl;
#endif
                vm.push(av >= bv);
                break;
            case OpCode::EQ:
#ifdef _DEBUG
                std::cerr << "[DEBUG][int64_t EQ] a=" << av << ", b=" << bv << ", result=" << (av == bv) << std::endl;
#endif
                vm.push(av == bv);
                break;
            case OpCode::NEQ:
#ifdef _DEBUG
                std::cerr << "[DEBUG][int64_t NEQ] a=" << av << ", b=" << bv << ", result=" << (av != bv) << std::endl;
#endif
                vm.push(av != bv);
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
#ifdef _DEBUG
                    std::cerr << "Division by zero (float)" << std::endl;
#endif
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
#ifdef _DEBUG
                    std::cerr << "Modulo by zero (float)" << std::endl;
#endif
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
#ifdef _DEBUG
                    std::cerr << "Floor division by zero (float)" << std::endl;
#endif
                    vm.push(std::monostate{});
                }
                else
                {
                    vm.push(std::floor(av / bv));
                }
                break;
            // --- So sánh ---
            case OpCode::LT:
                vm.push(av < bv);
                break;
            case OpCode::LTE:
                vm.push(av <= bv);
                break;
            case OpCode::GT:
                vm.push(av > bv);
                break;
            case OpCode::GTE:
                vm.push(av >= bv);
                break;
            case OpCode::EQ:
                vm.push(av == bv);
                break;
            case OpCode::NEQ:
                vm.push(av != bv);
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
#ifdef _DEBUG
                    std::cerr << "Division by zero (float/uint)" << std::endl;
#endif
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
#ifdef _DEBUG
                    std::cerr << "Modulo by zero (float/uint)" << std::endl;
#endif
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
#ifdef _DEBUG
                    std::cerr << "Floor division by zero (float/uint)" << std::endl;
#endif
                    vm.push(std::monostate{});
                }
                else
                {
                    vm.push(std::floor(av / bv));
                }
                break;
            // --- So sánh ---
            case OpCode::LT:
                vm.push(av < bv);
                break;
            case OpCode::LTE:
                vm.push(av <= bv);
                break;
            case OpCode::GT:
                vm.push(av > bv);
                break;
            case OpCode::GTE:
                vm.push(av >= bv);
                break;
            case OpCode::EQ:
                vm.push(av == bv);
                break;
            case OpCode::NEQ:
                vm.push(av != bv);
                break;
            default:
                break;
            }
        }
        else if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b))
        {
            const std::string &av = std::get<std::string>(a);
            const std::string &bv = std::get<std::string>(b);
            switch (instr.opcode)
            {
            case OpCode::EQ:
                vm.push(av == bv);
                break;
            case OpCode::NEQ:
                vm.push(av != bv);
                break;
            case OpCode::LT:
                vm.push(av < bv);
                break;
            case OpCode::LTE:
                vm.push(av <= bv);
                break;
            case OpCode::GT:
                vm.push(av > bv);
                break;
            case OpCode::GTE:
                vm.push(av >= bv);
                break;
            default:
                vm.push(std::monostate{});
                break;
            }
        }
        else if (std::holds_alternative<bool>(a) && std::holds_alternative<bool>(b))
        {
            bool av = std::get<bool>(a);
            bool bv = std::get<bool>(b);
            switch (instr.opcode)
            {
            case OpCode::EQ:
                vm.push(av == bv);
                break;
            case OpCode::NEQ:
                vm.push(av != bv);
                break;
            default:
                vm.push(std::monostate{});
                break;
            }
        }
        else
        {
#ifdef _DEBUG
            std::cerr << "Invalid operand types for arithmetic or comparison" << std::endl;
#endif
            vm.push(std::monostate{});
        }
    }
}
