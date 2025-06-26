#pragma once
#include <variant>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>

namespace Linh
{
    struct Value; // Forward declaration
    using Array = std::vector<Value>;
    using Map = std::unordered_map<std::string, Value>;

    struct Value : public std::variant<
                       std::monostate,
                       int64_t,
                       double,
                       std::string,
                       bool,
                       Array,
                       Map>
    {
        using variant::variant;
        Value() : variant() {}
        Value(const variant &v) : variant(v) {}
    };
}
