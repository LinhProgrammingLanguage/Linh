#include <unordered_map>
#include <string>
#include <memory>
#include <cmath>
#include <functional>
#include "LiVM/Value/Value.hpp"
#include "LiVM/LiVM.hpp"

// Define M_PI if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Linh
{
    namespace LiPM
    {
        // Function type for math functions
        using MathFunction = std::function<Value(const Value&)>;
        
        // Default packages registry
        static std::unordered_map<std::string, std::unordered_map<std::string, Value>> default_packages;
        
        // Math functions registry
        static std::unordered_map<std::string, MathFunction> math_functions;

        // Math function implementations
        static Value math_abs(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                int64_t v = std::get<int64_t>(val);
                return v < 0 ? -v : v;
            }
            else if (std::holds_alternative<double>(val))
            {
                return std::abs(std::get<double>(val));
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                return val; // uint64_t is always positive
            }
            else
            {
                return Value{}; // Return sol for unsupported types
            }
        }

        static Value math_ceil(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                return val; // int64_t is already "ceiled"
            }
            else if (std::holds_alternative<double>(val))
            {
                return std::ceil(std::get<double>(val));
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                return val; // uint64_t is already "ceiled"
            }
            else
            {
                return Value{}; // Return sol for unsupported types
            }
        }

        static Value math_floor(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                return val; // int64_t is already "floored"
            }
            else if (std::holds_alternative<double>(val))
            {
                return std::floor(std::get<double>(val));
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                return val; // uint64_t is already "floored"
            }
            else
            {
                return Value{}; // Return sol for unsupported types
            }
        }

        static Value math_round(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                return val; // int64_t is already "rounded"
            }
            else if (std::holds_alternative<double>(val))
            {
                return std::round(std::get<double>(val));
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                return val; // uint64_t is already "rounded"
            }
            else
            {
                return Value{}; // Return sol for unsupported types
            }
        }

        static Value math_trunc(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                return val; // int64_t is already "truncated"
            }
            else if (std::holds_alternative<double>(val))
            {
                return std::trunc(std::get<double>(val));
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                return val; // uint64_t is already "truncated"
            }
            else
            {
                return Value{}; // Return sol for unsupported types
            }
        }

        // Trigonometric functions
        static Value math_sin(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                return std::sin(static_cast<double>(std::get<int64_t>(val)));
            }
            else if (std::holds_alternative<double>(val))
            {
                return std::sin(std::get<double>(val));
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                return std::sin(static_cast<double>(std::get<uint64_t>(val)));
            }
            else
            {
                return Value{}; // Return sol for unsupported types
            }
        }

        static Value math_cos(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                return std::cos(static_cast<double>(std::get<int64_t>(val)));
            }
            else if (std::holds_alternative<double>(val))
            {
                return std::cos(std::get<double>(val));
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                return std::cos(static_cast<double>(std::get<uint64_t>(val)));
            }
            else
            {
                return Value{}; // Return sol for unsupported types
            }
        }

        static Value math_tan(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                return std::tan(static_cast<double>(std::get<int64_t>(val)));
            }
            else if (std::holds_alternative<double>(val))
            {
                return std::tan(std::get<double>(val));
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                return std::tan(static_cast<double>(std::get<uint64_t>(val)));
            }
            else
            {
                return Value{}; // Return sol for unsupported types
            }
        }

        static Value math_asin(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                double v = static_cast<double>(std::get<int64_t>(val));
                if (v < -1.0 || v > 1.0)
                    return Value{}; // Return sol for invalid input
                return std::asin(v);
            }
            else if (std::holds_alternative<double>(val))
            {
                double v = std::get<double>(val);
                if (v < -1.0 || v > 1.0)
                    return Value{}; // Return sol for invalid input
                return std::asin(v);
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                double v = static_cast<double>(std::get<uint64_t>(val));
                if (v > 1.0)
                    return Value{}; // Return sol for invalid input
                return std::asin(v);
            }
            else
            {
                return Value{}; // Return sol for unsupported types
            }
        }

        static Value math_acos(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                double v = static_cast<double>(std::get<int64_t>(val));
                if (v < -1.0 || v > 1.0)
                    return Value{}; // Return sol for invalid input
                return std::acos(v);
            }
            else if (std::holds_alternative<double>(val))
            {
                double v = std::get<double>(val);
                if (v < -1.0 || v > 1.0)
                    return Value{}; // Return sol for invalid input
                return std::acos(v);
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                double v = static_cast<double>(std::get<uint64_t>(val));
                if (v > 1.0)
                    return Value{}; // Return sol for invalid input
                return std::acos(v);
            }
            else
            {
                return Value{}; // Return sol for unsupported types
            }
        }

        static Value math_atan(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                return std::atan(static_cast<double>(std::get<int64_t>(val)));
            }
            else if (std::holds_alternative<double>(val))
            {
                return std::atan(std::get<double>(val));
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                return std::atan(static_cast<double>(std::get<uint64_t>(val)));
            }
            else
            {
                return Value{}; // Return sol for unsupported types
            }
        }

        static Value math_radians(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                return std::get<int64_t>(val) * M_PI / 180.0;
            }
            else if (std::holds_alternative<double>(val))
            {
                return std::get<double>(val) * M_PI / 180.0;
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                return std::get<uint64_t>(val) * M_PI / 180.0;
            }
            else
            {
                return Value{}; // Return sol for unsupported types
            }
        }

        // atan2 requires special handling as it needs 2 arguments
        // This will be handled directly in LiVM.cpp

        // Hyperbolic functions
        static Value math_sinh(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
                return std::sinh(static_cast<double>(std::get<int64_t>(val)));
            else if (std::holds_alternative<double>(val))
                return std::sinh(std::get<double>(val));
            else if (std::holds_alternative<uint64_t>(val))
                return std::sinh(static_cast<double>(std::get<uint64_t>(val)));
            else
                return Value{};
        }
        static Value math_cosh(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
                return std::cosh(static_cast<double>(std::get<int64_t>(val)));
            else if (std::holds_alternative<double>(val))
                return std::cosh(std::get<double>(val));
            else if (std::holds_alternative<uint64_t>(val))
                return std::cosh(static_cast<double>(std::get<uint64_t>(val)));
            else
                return Value{};
        }
        static Value math_tanh(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
                return std::tanh(static_cast<double>(std::get<int64_t>(val)));
            else if (std::holds_alternative<double>(val))
                return std::tanh(std::get<double>(val));
            else if (std::holds_alternative<uint64_t>(val))
                return std::tanh(static_cast<double>(std::get<uint64_t>(val)));
            else
                return Value{};
        }
        static Value math_asinh(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
                return std::asinh(static_cast<double>(std::get<int64_t>(val)));
            else if (std::holds_alternative<double>(val))
                return std::asinh(std::get<double>(val));
            else if (std::holds_alternative<uint64_t>(val))
                return std::asinh(static_cast<double>(std::get<uint64_t>(val)));
            else
                return Value{};
        }
        static Value math_acosh(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
                return std::acosh(static_cast<double>(std::get<int64_t>(val)));
            else if (std::holds_alternative<double>(val))
                return std::acosh(std::get<double>(val));
            else if (std::holds_alternative<uint64_t>(val))
                return std::acosh(static_cast<double>(std::get<uint64_t>(val)));
            else
                return Value{};
        }
        static Value math_atanh(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
                return std::atanh(static_cast<double>(std::get<int64_t>(val)));
            else if (std::holds_alternative<double>(val))
                return std::atanh(std::get<double>(val));
            else if (std::holds_alternative<uint64_t>(val))
                return std::atanh(static_cast<double>(std::get<uint64_t>(val)));
            else
                return Value{};
        }

        // Additional mathematical functions
        static Value math_pow(const Value& val)
        {
            // This will be handled specially in LiVM.cpp for 2 arguments
            return Value{};
        }
        
        static Value math_sqrt(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                int64_t v = std::get<int64_t>(val);
                if (v < 0) return Value{}; // Return sol for negative numbers
                return std::sqrt(static_cast<double>(v));
            }
            else if (std::holds_alternative<double>(val))
            {
                double v = std::get<double>(val);
                if (v < 0) return Value{}; // Return sol for negative numbers
                return std::sqrt(v);
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                return std::sqrt(static_cast<double>(std::get<uint64_t>(val)));
            }
            else
            {
                return Value{};
            }
        }
        
        static Value math_cbrt(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                return std::cbrt(static_cast<double>(std::get<int64_t>(val)));
            }
            else if (std::holds_alternative<double>(val))
            {
                return std::cbrt(std::get<double>(val));
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                return std::cbrt(static_cast<double>(std::get<uint64_t>(val)));
            }
            else
            {
                return Value{};
            }
        }
        
        static Value math_exp(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                return std::exp(static_cast<double>(std::get<int64_t>(val)));
            }
            else if (std::holds_alternative<double>(val))
            {
                return std::exp(std::get<double>(val));
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                return std::exp(static_cast<double>(std::get<uint64_t>(val)));
            }
            else
            {
                return Value{};
            }
        }
        
        static Value math_expm1(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                return std::expm1(static_cast<double>(std::get<int64_t>(val)));
            }
            else if (std::holds_alternative<double>(val))
            {
                return std::expm1(std::get<double>(val));
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                return std::expm1(static_cast<double>(std::get<uint64_t>(val)));
            }
            else
            {
                return Value{};
            }
        }
        
        static Value math_log(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                int64_t v = std::get<int64_t>(val);
                if (v <= 0) return Value{}; // Return sol for non-positive numbers
                return std::log(static_cast<double>(v));
            }
            else if (std::holds_alternative<double>(val))
            {
                double v = std::get<double>(val);
                if (v <= 0) return Value{}; // Return sol for non-positive numbers
                return std::log(v);
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                uint64_t v = std::get<uint64_t>(val);
                if (v == 0) return Value{}; // Return sol for zero
                return std::log(static_cast<double>(v));
            }
            else
            {
                return Value{};
            }
        }
        
        static Value math_log1p(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                int64_t v = std::get<int64_t>(val);
                if (v <= -1) return Value{}; // Return sol for invalid input
                return std::log1p(static_cast<double>(v));
            }
            else if (std::holds_alternative<double>(val))
            {
                double v = std::get<double>(val);
                if (v <= -1) return Value{}; // Return sol for invalid input
                return std::log1p(v);
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                return std::log1p(static_cast<double>(std::get<uint64_t>(val)));
            }
            else
            {
                return Value{};
            }
        }
        
        static Value math_log10(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                int64_t v = std::get<int64_t>(val);
                if (v <= 0) return Value{}; // Return sol for non-positive numbers
                return std::log10(static_cast<double>(v));
            }
            else if (std::holds_alternative<double>(val))
            {
                double v = std::get<double>(val);
                if (v <= 0) return Value{}; // Return sol for non-positive numbers
                return std::log10(v);
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                uint64_t v = std::get<uint64_t>(val);
                if (v == 0) return Value{}; // Return sol for zero
                return std::log10(static_cast<double>(v));
            }
            else
            {
                return Value{};
            }
        }
        
        static Value math_log2(const Value& val)
        {
            if (std::holds_alternative<int64_t>(val))
            {
                int64_t v = std::get<int64_t>(val);
                if (v <= 0) return Value{}; // Return sol for non-positive numbers
                return std::log2(static_cast<double>(v));
            }
            else if (std::holds_alternative<double>(val))
            {
                double v = std::get<double>(val);
                if (v <= 0) return Value{}; // Return sol for non-positive numbers
                return std::log2(v);
            }
            else if (std::holds_alternative<uint64_t>(val))
            {
                uint64_t v = std::get<uint64_t>(val);
                if (v == 0) return Value{}; // Return sol for zero
                return std::log2(static_cast<double>(v));
            }
            else
            {
                return Value{};
            }
        }

        // Initialize default packages and functions
        void initialize_default_packages()
        {
            // Math package
            std::unordered_map<std::string, Value> math_package;
            
            // Mathematical constants with appropriate precision for double
            math_package["pi"] = 3.141592653589793;
            math_package["e"] = 2.718281828459045;   // e (15 digits)
            math_package["tau"] = 6.283185307179586; // τ (15 digits)
            math_package["phi"] = 1.618033988749895; // φ (golden ratio, 15 digits)
            
            default_packages["math"] = std::move(math_package);
            
            // Initialize math functions
            math_functions["abs"] = math_abs;
            math_functions["ceil"] = math_ceil;
            math_functions["floor"] = math_floor;
            math_functions["round"] = math_round;
            math_functions["trunc"] = math_trunc;
            
            // Initialize trigonometric functions
            math_functions["sin"] = math_sin;
            math_functions["cos"] = math_cos;
            math_functions["tan"] = math_tan;
            math_functions["asin"] = math_asin;
            math_functions["acos"] = math_acos;
            math_functions["atan"] = math_atan;
            math_functions["radians"] = math_radians;

            // Initialize hyperbolic functions
            math_functions["sinh"] = math_sinh;
            math_functions["cosh"] = math_cosh;
            math_functions["tanh"] = math_tanh;
            math_functions["asinh"] = math_asinh;
            math_functions["acosh"] = math_acosh;
            math_functions["atanh"] = math_atanh;

            // Initialize additional mathematical functions
            math_functions["sqrt"] = math_sqrt;
            math_functions["cbrt"] = math_cbrt;
            math_functions["exp"] = math_exp;
            math_functions["expm1"] = math_expm1;
            math_functions["log"] = math_log;
            math_functions["log1p"] = math_log1p;
            math_functions["log10"] = math_log10;
            math_functions["log2"] = math_log2;
        }

        // Get a package by name
        const std::unordered_map<std::string, Value>* get_package(const std::string& package_name)
        {
            if (default_packages.empty())
            {
                initialize_default_packages();
            }
            
            auto it = default_packages.find(package_name);
            if (it != default_packages.end())
            {
                return &(it->second);
            }
            return nullptr;
        }

        // Get a specific constant from a package
        Value get_constant(const std::string& package_name, const std::string& constant_name)
        {
            const auto* package = get_package(package_name);
            if (package)
            {
                auto it = package->find(constant_name);
                if (it != package->end())
                {
                    return it->second;
                }
            }
            return Value{}; // Return sol if not found
        }

        // Get a math function by name
        MathFunction get_math_function(const std::string& function_name)
        {
            if (math_functions.empty())
            {
                initialize_default_packages();
            }
            
            auto it = math_functions.find(function_name);
            if (it != math_functions.end())
            {
                return it->second;
            }
            return nullptr; // Return null function if not found
        }

        // Check if a package exists
        bool package_exists(const std::string& package_name)
        {
            if (default_packages.empty())
            {
                initialize_default_packages();
            }
            return default_packages.find(package_name) != default_packages.end();
        }

        // Get all available package names
        std::vector<std::string> get_available_packages()
        {
            if (default_packages.empty())
            {
                initialize_default_packages();
            }
            
            std::vector<std::string> package_names;
            for (const auto& pair : default_packages)
            {
                package_names.push_back(pair.first);
            }
            return package_names;
        }

        // Get all constants in a package
        std::vector<std::string> get_package_constants(const std::string& package_name)
        {
            const auto* package = get_package(package_name);
            if (package)
            {
                std::vector<std::string> constant_names;
                for (const auto& pair : *package)
                {
                    constant_names.push_back(pair.first);
                }
                return constant_names;
            }
            return {};
        }

        // Get all functions in math package
        std::vector<std::string> get_math_functions()
        {
            if (math_functions.empty())
            {
                initialize_default_packages();
            }
            
            std::vector<std::string> function_names;
            for (const auto& pair : math_functions)
            {
                function_names.push_back(pair.first);
            }
            return function_names;
        }
    }
}
