#include "iostream.hpp"
#include "../type.hpp"

namespace LinhIO
{

    void linh_print(const Linh::Value &val)
    {
        std::cout << std::boolalpha << Linh::to_str(val) << std::endl;
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
