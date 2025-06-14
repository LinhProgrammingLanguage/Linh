#include "iostream.hpp"

namespace LinhIO
{

    void linh_print(const std::variant<int64_t, double, std::string, bool> &val)
    {
        std::cout << std::boolalpha;
        std::visit([](auto &&arg)
                   { std::cout << arg << std::endl; }, val);
    }

    std::string linh_input(const std::string &prompt)
    {
        if (!prompt.empty())
            std::cout << prompt;
        std::string input_val;
        std::getline(std::cin, input_val);
        return input_val;
    }

}
