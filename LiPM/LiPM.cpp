#include <unordered_map>
#include <string>
#include <memory>
#include <cmath>
#include <functional>
#include "LiVM/Value/Value.hpp"
#include "LiVM/LiVM.hpp"
#include "../config.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Linh
{
    namespace LiPM
    {
        using MathFunction = std::function<Value(const Value&)>;
        static std::unordered_map<std::string, std::unordered_map<std::string, Value>> default_packages;

        void initialize_default_packages()
        {
            for (const auto& pkg : linh_packages) {
                if (pkg == "math") {
                    std::unordered_map<std::string, Value> math_package;
                    math_package["pi"] = 3.141592653589793;
                    math_package["e"] = 2.718281828459045;
                    math_package["tau"] = 6.283185307179586;
                    math_package["phi"] = 1.618033988749895;
                    default_packages["math"] = std::move(math_package);
                } else if (pkg == "time") {
                    std::unordered_map<std::string, Value> time_package;
                    time_package["time"] = []() -> Value {
                        using namespace std::chrono;
                        auto now = std::chrono::system_clock::now();
                        auto duration = now.time_since_epoch();
                        double seconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count() / 1e6;
                        return Value(seconds);
                    }();
                    default_packages["time"] = std::move(time_package);
                }
                // Có thể mở rộng thêm các package khác ở đây
            }
        }

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

        bool package_exists(const std::string& package_name)
        {
            if (default_packages.empty())
            {
                initialize_default_packages();
            }
            return default_packages.find(package_name) != default_packages.end();
        }

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