#pragma once
#include "LiVM.hpp"
#include <variant>
#include <string>
#include <cstdint>

namespace Linh
{
    using Value = std::variant<std::monostate, int64_t, double, std::string, bool>;

    std::string type_of(const Value &val);

    // Hàm chuyển đổi kiểu dữ liệu
    std::string to_str(const Value &val);
    int64_t to_int(const Value &val);
    double to_float(const Value &val);
    uint64_t to_uint(const Value &val);
    bool to_bool(const Value &val);

    // Định nghĩa hàm type() cho LiVM
    void type(LiVM &vm);
}
