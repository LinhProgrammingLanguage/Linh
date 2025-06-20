#pragma once
#include <variant>
#include <string>
#include <iostream>

namespace LinhIO
{
    void linh_print(const std::variant<std::monostate, int64_t, double, std::string, bool> &val);
    std::string linh_input(const std::string &prompt);
}
