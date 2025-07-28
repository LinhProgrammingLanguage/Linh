#pragma once
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include "LiVM/Value/Value.hpp"

// Forward declarations
namespace Linh {
    class LiVM;
    using BytecodeChunk = std::vector<struct Instruction>;
    struct Instruction;
}

namespace Linh {
    // Thêm struct để lưu thông tin tham số
    struct FunctionParameter {
        std::string name;
        std::optional<std::string> type; // Kiểu dữ liệu (nếu có)
        bool is_static; // true nếu có 'vas' keyword
        
        FunctionParameter(const std::string& n, const std::optional<std::string>& t = std::nullopt, bool static_ = false)
            : name(n), type(t), is_static(static_) {}
    };

    struct FunctionObject {
        std::string name;
        std::vector<FunctionParameter> params;
        BytecodeChunk body; // Thân hàm dưới dạng bytecode
        // TODO: Thêm trường cho closure/environment nếu cần
    };

    using FunctionPtr = std::shared_ptr<FunctionObject>;

    // Tạo function object
    FunctionPtr create_function(const std::string& name, const std::vector<FunctionParameter>& params, const BytecodeChunk& body);
    // Gọi function object
    Value call_function(FunctionPtr fn, const std::vector<Value>& args, LiVM& vm);
} 