#pragma once
#include <variant>
#include <string>
#include <iostream>
#include "../Value/Value.hpp"

namespace LinhIO
{
    void linh_print(const Linh::Value &val);
    std::string linh_input(const std::string &prompt);
    void linh_printf(const Linh::Value &val); // Thêm hàm printf không tự động xuống dòng
}
