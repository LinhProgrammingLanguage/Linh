#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "../LiVM/Value/Value.hpp"

namespace Linh
{
    namespace LiPM
    {
        // Initialize default packages
        void initialize_default_packages();

        // Get a package by name
        const std::unordered_map<std::string, Value>* get_package(const std::string& package_name);

        // Get a specific constant from a package
        Value get_constant(const std::string& package_name, const std::string& constant_name);

        // Check if a package exists
        bool package_exists(const std::string& package_name);

        // Get all available package names
        std::vector<std::string> get_available_packages();

        // Get all constants in a package
        std::vector<std::string> get_package_constants(const std::string& package_name);
    }
} 