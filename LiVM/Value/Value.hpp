#pragma once
#include <variant>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <memory> // Thêm dòng này cho shared_ptr

namespace Linh
{
    struct Value; // Forward declaration
    using Array = std::shared_ptr<std::vector<Value>>;
    using Map = std::shared_ptr<std::unordered_map<std::string, Value>>;

    struct Value : public std::variant<
                       std::monostate,
                       int64_t,
                       uint64_t,
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
