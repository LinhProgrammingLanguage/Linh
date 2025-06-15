#include "REPL.hpp"
#include "LinhC/Parsing/Lexer/Lexer.hpp"
#include "LinhC/Parsing/Parser/Parser.hpp"
#include "LinhC/Parsing/Semantic/SemanticAnalyzer.hpp"
#include "LinhC/Bytecode/BytecodeEmitter.hpp"
#include "LiVM/LiVM.hpp"
#include <iostream>
#include <string>

namespace Linh
{
    void run_repl()
    {
        std::string line;
        std::vector<std::string> repl_lines;
        std::vector<Linh::AST::StmtPtr> global_stmts;
        Linh::LiVM vm;
        Linh::BytecodeEmitter emitter;
        std::unordered_map<std::string, Linh::BytecodeEmitter::FunctionInfo> function_table;

        while (true)
        {
            std::cout << ">>> ";
            if (!std::getline(std::cin, line))
                break;
            if (line == ".exit" || line == ".quit")
                break;
            if (line.empty())
                continue;

            // --- Multi-line literal support ---
            std::string full_stmt = line;
            // Nếu dòng kết thúc bằng { hoặc [ (sau khoảng trắng), tiếp tục đọc cho đến khi gặp dòng chỉ chứa } hoặc ]
            auto trim = [](const std::string &s)
            {
                size_t start = s.find_first_not_of(" \t\r\n");
                size_t end = s.find_last_not_of(" \t\r\n");
                if (start == std::string::npos)
                    return std::string();
                return s.substr(start, end - start + 1);
            };
            std::string trimmed = trim(line);
            bool multiline = false;
            if (!trimmed.empty() && (trimmed.back() == '{' || trimmed.back() == '['))
            {
                multiline = true;
            }
            if (multiline)
            {
                std::string closing = (trimmed.back() == '{') ? "}" : "]";
                while (true)
                {
                    std::cout << "... ";
                    std::string next_line;
                    if (!std::getline(std::cin, next_line))
                        break;
                    full_stmt += "\n" + next_line;
                    std::string next_trim = trim(next_line);
                    if (next_trim == closing)
                        break;
                }
            }

            repl_lines.push_back(full_stmt);
            std::string source;
            for (const auto &l : repl_lines)
                source += l + "\n";

            Linh::Lexer lexer(source);
            auto tokens = lexer.scan_tokens();

            Linh::Parser parser(tokens);
            auto stmts = parser.parse();

            Linh::Semantic::SemanticAnalyzer analyzer;
            analyzer.analyze(stmts, false); // REPL mode: giữ lại scope

            if (!analyzer.errors.empty())
            {
                for (const auto &err : analyzer.errors)
                {
                    std::cerr << "[SemanticError] " << err.message << " (line " << err.line << ", col " << err.column << ")\n";
                }
                continue;
            }

            emitter.emit(stmts);
            function_table = emitter.get_functions();

            // Chuyển đổi function_table sang LiVM::Function
            std::unordered_map<std::string, Linh::LiVM::Function> vm_funcs;
            for (const auto &[name, finfo] : function_table)
            {
                Linh::LiVM::Function f;
                f.code = finfo.code;
                f.param_names = finfo.param_names;
                vm_funcs[name] = std::move(f);
            }

            // Cập nhật function table cho VM
            vm.set_functions(vm_funcs);

            const auto &chunk = emitter.get_chunk();
            vm.run(chunk);
        }
    }
}
