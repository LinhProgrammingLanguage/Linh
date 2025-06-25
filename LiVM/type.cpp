#include "type.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <limits>

namespace Linh
{
    void type(LiVM &vm)
    {
        vm.type();
    }

    std::string type_of(const Value &val)
    {
        if (std::holds_alternative<std::monostate>(val))
            return "uninit";
        if (std::holds_alternative<int64_t>(val))
            return "int";
        if (std::holds_alternative<double>(val))
            return "float";
        if (std::holds_alternative<std::string>(val))
            return "str";
        if (std::holds_alternative<bool>(val))
            return "bool";
        return "unknown";
    }

    std::string to_str(const Value &val)
    {
        if (std::holds_alternative<std::monostate>(val))
            return "uninit";
        if (std::holds_alternative<std::string>(val))
            return std::get<std::string>(val);
        if (std::holds_alternative<int64_t>(val))
            return std::to_string(std::get<int64_t>(val));
        if (std::holds_alternative<double>(val))
        {
            std::ostringstream oss;
            oss << std::get<double>(val);
            return oss.str();
        }
        if (std::holds_alternative<bool>(val))
            return std::get<bool>(val) ? "true" : "false";
        return "";
    }

    int64_t to_int(const Value &val)
    {
        if (std::holds_alternative<int64_t>(val))
            return std::get<int64_t>(val);
        if (std::holds_alternative<double>(val))
            return static_cast<int64_t>(std::get<double>(val));
        if (std::holds_alternative<std::string>(val))
        {
            try
            {
                return std::stoll(std::get<std::string>(val));
            }
            catch (...)
            {
                return 0;
            }
        }
        if (std::holds_alternative<bool>(val))
            return std::get<bool>(val) ? 1 : 0;
        return 0;
    }

    double to_float(const Value &val)
    {
        if (std::holds_alternative<double>(val))
            return std::get<double>(val);
        if (std::holds_alternative<int64_t>(val))
            return static_cast<double>(std::get<int64_t>(val));
        if (std::holds_alternative<std::string>(val))
        {
            try
            {
                return std::stod(std::get<std::string>(val));
            }
            catch (...)
            {
                return 0.0;
            }
        }
        if (std::holds_alternative<bool>(val))
            return std::get<bool>(val) ? 1.0 : 0.0;
        return 0.0;
    }

    uint64_t to_uint(const Value &val)
    {
        if (std::holds_alternative<int64_t>(val))
            return static_cast<uint64_t>(std::max<int64_t>(0, std::get<int64_t>(val)));
        if (std::holds_alternative<double>(val))
            return static_cast<uint64_t>(std::max<double>(0.0, std::get<double>(val)));
        if (std::holds_alternative<std::string>(val))
        {
            try
            {
                auto v = std::stoull(std::get<std::string>(val));
                return v;
            }
            catch (...)
            {
                return 0;
            }
        }
        if (std::holds_alternative<bool>(val))
            return std::get<bool>(val) ? 1 : 0;
        return 0;
    }

    bool to_bool(const Value &val)
    {
        if (std::holds_alternative<bool>(val))
            return std::get<bool>(val);
        if (std::holds_alternative<int64_t>(val))
            return std::get<int64_t>(val) != 0;
        if (std::holds_alternative<double>(val))
            return std::get<double>(val) != 0.0;
        if (std::holds_alternative<std::string>(val))
            return !std::get<std::string>(val).empty();
        return false;
    }
}
