#pragma once
#include "Value/Value.hpp"
#include "LiVM.hpp"
#include <string>
#include <cstdint>

namespace Linh
{
    using Value = Linh::Value;
    using Array = Linh::Array;
    using Map = Linh::Map;

    std::string type_of(const Value &val);

    // Hàm format số thực theo quy tắc của Linh
    std::string format_float_linh(double value);

    // Hàm chuyển đổi kiểu dữ liệu
    std::string to_str(const Value &val);
    int64_t to_int(const Value &val);
    double to_float(const Value &val);
    uint64_t to_uint(const Value &val);
    bool to_bool(const Value &val);

    // Định nghĩa hàm type() cho LiVM
    void type(LiVM &vm);

    // Hàm lấy độ dài cho array, map, string
    int64_t len(const Value &val);
}
