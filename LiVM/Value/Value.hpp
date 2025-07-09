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

    using VariantType = std::variant<
        std::monostate,
        bool,
        int64_t,
        uint64_t,
        double,
        std::string,
        Array,
        Map>;

    struct Value : public VariantType {
        using VariantType::VariantType;
        Value() : VariantType() {}
        Value(const VariantType &v) : VariantType(v) {}
    };
}
