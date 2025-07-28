#include "Func.hpp"
#include "LiVM/LiVM.hpp"
#include "LinhC/Bytecode/Bytecode.hpp"
#include <iostream>

namespace Linh {
    // Tạo function object
    FunctionPtr create_function(const std::string& name, const std::vector<FunctionParameter>& params, const BytecodeChunk& body) {
        auto fn = std::make_shared<FunctionObject>();
        fn->name = name;
        fn->params = params;
        fn->body = body; // Lưu thân hàm
        return fn;
    }

    // Gọi function object
    Value call_function(FunctionPtr fn, const std::vector<Value>& args, LiVM& vm) {
        // Kiểm tra số lượng tham số
        if (args.size() != fn->params.size()) {
            std::cerr << "Error: Function " << fn->name << " expects "
                      << fn->params.size() << " arguments, but got " << args.size() << std::endl;
            return Value(); // Trả về giá trị mặc định
        }

        // Lưu trạng thái hiện tại của VM
        auto original_stack = vm.stack;
        auto original_vars = vm.variables;
        auto original_call_stack = vm.call_stack;
        auto original_ip = vm.ip; // Save original IP

        // Tạo môi trường local cho function
        vm.variables.clear();

        // Bind tham số với giá trị
        for (size_t i = 0; i < fn->params.size(); ++i) {
            // TODO: Thêm type checking cho static parameters (vas)
            vm.variables[i] = args[i];
        }

        // Lưu return address
        vm.call_stack.push_back({vm.ip + 1, vm.variables});

        // Thực thi thân hàm
        vm.ip = 0; // Reset IP for function's own chunk
        vm.run_chunk(fn->body);

        // Lấy kết quả từ stack
        Value result = Value(); // Default value
        if (!vm.stack.empty()) {
            result = vm.stack.back();
            vm.stack.pop_back();
        }

        // Khôi phục trạng thái VM
        vm.stack = original_stack;
        vm.variables = original_vars;
        vm.call_stack = original_call_stack;
        vm.ip = original_ip; // Restore original IP

        return result;
    }
}
