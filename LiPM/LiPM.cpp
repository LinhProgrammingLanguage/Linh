#include <unordered_map>
#include <string>
#include <memory>
#include "LiVM/Value/Value.hpp"
#include "LiVM/LiVM.hpp"

namespace Linh
{
    namespace LiPM
    {
        // Default packages registry
        static std::unordered_map<std::string, std::unordered_map<std::string, Value>> default_packages;

        // Initialize default packages
        void initialize_default_packages()
        {
            // Math package
            std::unordered_map<std::string, Value> math_package;
            
            // Mathematical constants with appropriate precision for double
            math_package["pi"] = 3.141592653589793;  // π (15 digits)
            math_package["e"] = 2.718281828459045;   // e (15 digits)
            math_package["tau"] = 6.283185307179586; // τ (15 digits)
            
            default_packages["math"] = std::move(math_package);
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
    }
}
