#include "LinhC/Parsing/Lexer/Lexer.hpp"
#include "LinhC/Parsing/Parser/Parser.hpp"
#include "LinhC/Parsing/AST/ASTPrinter.hpp" // For printing AST
#include "LinhC/Parsing/Semantic/SemanticAnalyzer.hpp"
#include "LinhC/Bytecode/BytecodeEmitter.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string> // For std::string

int main(int argc, char **argv)
{
    std::string source_code;
    std::ifstream file("test.li"); // Try to read from file test.li

    if (file)
    {
        std::stringstream buffer;
        buffer << file.rdbuf();
        source_code = buffer.str();
        file.close();
        std::cout << "--- Loaded code from test.li ---" << std::endl;
    }
    else
    {
        std::cerr << "Cannot open file test.li. Using minimal default code." << std::endl;
        source_code = R"(
        print("Hello from default Linh code!");
        var x = 10;
        if (x > 5) {
            print("x is greater than 5");
        } else {
            print("x is not greater than 5");
        }
    )";
    }

    std::cout << "--- Source Code Being Parsed ---\n"
              << source_code << "\n--------------------------------\n";

    Linh::Lexer lexer(source_code);
    std::vector<Linh::Token> tokens = lexer.scan_tokens();

    std::cout << "\n--- Tokens ---" << std::endl;
    bool lexer_has_error = false;
    for (const auto &token : tokens)
    {
        std::cout << token.to_string() << std::endl;
        if (token.type == Linh::TokenType::ERROR)
        {
            lexer_has_error = true;
        }
    }

    if (lexer_has_error)
    {
        std::cerr << "\nLexer error, aborting parse." << std::endl;
        return 1;
    }

    std::cout << "\n--- Abstract Syntax Tree (AST) ---" << std::endl;
    Linh::Parser parser(tokens);
    Linh::AST::StmtList ast_statements = parser.parse();

    if (parser.had_error())
    {
        std::cerr << "\nParser encountered errors. AST may be incomplete or empty." << std::endl;
        // Still try to print AST to see how far it parsed (if ast_statements is not empty)
    }

    if (!ast_statements.empty() || !parser.had_error())
    { // Print AST if no error or if there is something to print
        Linh::AST::ASTPrinter ast_printer;
        std::cout << ast_printer.print(ast_statements) << std::endl;
    }
    else if (parser.had_error() && ast_statements.empty())
    {
        std::cout << "(AST is empty due to parsing errors)" << std::endl;
    }

    // --- Semantic Analysis ---
    Linh::Semantic::SemanticAnalyzer sema;
    sema.analyze(ast_statements);
    if (!sema.errors.empty())
    {
        std::cerr << "\nSemantic errors:\n";
        for (const auto &err : sema.errors)
        {
            std::cerr << "[Line " << err.line << ", Col " << err.column << "] " << err.message << std::endl;
        }
        std::cout << "\nSemantic analysis failed due to errors." << std::endl;
        return 1;
    }

    if (parser.had_error())
    {
        std::cout << "\nParse failed due to parser errors." << std::endl;
        return 1;
    }

    // --- Bytecode ---
    Linh::BytecodeEmitter emitter;
    emitter.emit(ast_statements);

    // In kết quả bytecode (demo)
    const auto &chunk = emitter.get_chunk();
    for (const auto &instr : chunk)
    {
        std::cout << "Op: " << static_cast<int>(instr.opcode);
        // In toán hạng nếu có
        if (std::holds_alternative<int64_t>(instr.operand))
            std::cout << " " << std::get<int64_t>(instr.operand);
        else if (std::holds_alternative<double>(instr.operand))
            std::cout << " " << std::get<double>(instr.operand);
        else if (std::holds_alternative<std::string>(instr.operand))
            std::cout << " \"" << std::get<std::string>(instr.operand) << "\"";
        else if (std::holds_alternative<bool>(instr.operand))
            std::cout << " " << (std::get<bool>(instr.operand) ? "true" : "false");
        std::cout << std::endl;
    }

    std::cout << "\nParse succeeded!" << std::endl;
    return 0;
}