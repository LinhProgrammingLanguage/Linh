// NOTE: Tinh Linh does not allow the existence of null. Only 'uninit' is used as the primitive 'no-value'.

#include "LinhC/Parsing/Lexer/Lexer.hpp"
#include "LinhC/Parsing/Parser/Parser.hpp"
#include "LinhC/Parsing/AST/ASTPrinter.hpp" // For printing AST
#include "LinhC/Parsing/Semantic/SemanticAnalyzer.hpp"
#include "LinhC/Bytecode/BytecodeEmitter.hpp"
#include "LiVM/LiVM.hpp"
#include "REPL.hpp" // Thêm dòng này
#include <iostream>
#include <fstream>
#include <sstream>
#include <string> // For std::string
// utf 8
#ifdef _WIN32
#include <windows.h>
#else
#include <locale>
#endif

void force_console_utf8()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#else
    std::locale::global(std::locale(""));
    std::cout.imbue(std::locale());
#endif
}

void increase_performance()
{
    // Turn off sync with stdio to increase input/output speed
    std::ios_base::sync_with_stdio(false);

    // Turn off automatic flush of cout when cin needs input
    std::cin.tie(nullptr);
}

void runSource(const std::string &source_code,
               Linh::Semantic::SemanticAnalyzer *sema_ptr = nullptr,
               Linh::BytecodeEmitter *emitter_ptr = nullptr,
               Linh::LiVM *vm_ptr = nullptr);

void runFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Could not open file: " << filename << std::endl;
        return;
    }
    std::string line, source;
    while (std::getline(file, line))
    {
        source += line + "\n";
    }
    runSource(source, nullptr, nullptr, nullptr);
}

Linh::BytecodeEmitter *g_main_emitter = nullptr; // Thêm dòng này

void runSource(const std::string &source_code,
               Linh::Semantic::SemanticAnalyzer *sema_ptr,
               Linh::BytecodeEmitter *emitter_ptr,
               Linh::LiVM *vm_ptr)
{
    std::cout << "--- Source Code Being Parsed ---\n"
              << source_code << "\n--------------------------------\n";

    Linh::Lexer lexer(source_code);
    std::vector<Linh::Token> tokens = lexer.scan_tokens();
    Linh::Parser parser(tokens);
    Linh::AST::StmtList ast = parser.parse();
    if (parser.had_error())
    {
        std::cerr << "Lỗi cú pháp, dừng thực thi." << std::endl;
        return;
    }

    Linh::BytecodeEmitter emitter;
    g_main_emitter = &emitter; // Đặt emitter chính trước khi semantic để import có thể merge
    Linh::Semantic::SemanticAnalyzer sema;
    sema.analyze(ast);
    if (!sema.errors.empty())
    {
        for (const auto &err : sema.errors)
        {
            std::cerr << "[Line " << err.line << ", Col " << err.column << "] SemanticError : " << err.message << std::endl;
        }
        std::cerr << "Có lỗi semantic, dừng thực thi." << std::endl;
        return;
    }
    emitter.emit(ast);
    g_main_emitter = nullptr; // Đặt lại sau khi xong

    // --- Debug: In ra danh sách function sau khi merge ---
    std::cout << "\n--- Function Table After Import Merge ---\n";
    for (const auto &kv : emitter.get_functions())
    {
        std::cout << "Function: " << kv.first << ", param_count: " << kv.second.param_names.size() << "\n";
    }
    std::cout << "------------------------------------------\n";
    // -----------------------------------------------------

    // Nếu function table vẫn rỗng, debug thêm:
    if (emitter.get_functions().empty())
    {
        std::cout << "[DEBUG] Function table is empty after import. Possible reasons:\n";
        std::cout << "- File Li/add.li không tồn tại hoặc không đọc được\n";
        std::cout << "- Hàm add không được emit đúng trong BytecodeEmitter\n";
        std::cout << "- Quá trình merge function table trong visitImportStmt không thành công\n";
    }

    // --- Run VM ---
    Linh::LiVM vm;

    // --- Chuyển đổi function table ---
    std::unordered_map<std::string, Linh::LiVM::Function> vm_functions;
    for (const auto &kv : emitter.get_functions())
    {
        Linh::LiVM::Function fn;
        fn.code = kv.second.code;
        fn.param_names = kv.second.param_names;
        vm_functions[kv.first] = std::move(fn);
    }
    vm.set_functions(vm_functions);
    // --- Kết thúc chuyển đổi ---

    vm.run(emitter.get_chunk());

    // Debug: print VM stack and variables after execution (optional)
    // (You can add methods to LiVM to expose stack/vars for debugging if needed)

    std::cout << "\nParse succeeded!" << std::endl;
}

void runSource(const std::string &source_code)
{
    runSource(source_code, nullptr, nullptr, nullptr);
}

int main(int argc, char **argv)
{
    force_console_utf8();
    increase_performance();
    if (argc > 1)
    {
        runFile(argv[1]);
    }
    else
    {
        Linh::run_repl(); // Thay thế runREPL()
    }

    return 0;
}