#include "LinhC/Parsing/Lexer/Lexer.hpp"
#include "LinhC/Parsing/Parser/Parser.hpp"
#include "LinhC/Parsing/AST/ASTPrinter.hpp" // For printing AST
#include "LinhC/Parsing/Semantic/SemanticAnalyzer.hpp"
#include "LinhC/Bytecode/BytecodeEmitter.hpp"
#include "LiVM/LiVM.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string> // For std::string

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
    runSource(source);
}

void runREPL()
{
    std::string line;
    std::cout << "Linh REPL. Type 'exit' to quit." << std::endl;
    // --- Persist state across REPL lines ---
    static Linh::Semantic::SemanticAnalyzer sema;
    static Linh::BytecodeEmitter emitter;
    static Linh::LiVM vm;
    while (true)
    {
        std::cout << ">>> ";
        if (!std::getline(std::cin, line))
            break;
        if (line == "exit")
            break;
        runSource(line, &sema, &emitter, &vm);
    }
}

void runSource(const std::string &source_code,
               Linh::Semantic::SemanticAnalyzer *sema_ptr,
               Linh::BytecodeEmitter *emitter_ptr,
               Linh::LiVM *vm_ptr)
{
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
        return;
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
    Linh::Semantic::SemanticAnalyzer *sema;
    if (sema_ptr)
        sema = sema_ptr;
    else
        sema = new Linh::Semantic::SemanticAnalyzer();
    // --- Only reset state if not in REPL ---
    bool reset_state = (sema_ptr == nullptr);
    sema->analyze(ast_statements, reset_state);
    if (!sema->errors.empty())
    {
        std::cerr << "\nSemantic errors:\n";
        for (const auto &err : sema->errors)
        {
            std::cerr << "[Line " << err.line << ", Col " << err.column << "] " << err.message << std::endl;
        }
        std::cout << "\nSemantic analysis failed due to errors." << std::endl;
        if (!sema_ptr)
            delete sema;
        return;
    }

    if (parser.had_error())
    {
        std::cout << "\nParse failed due to parser errors." << std::endl;
        return;
    }

    // --- Bytecode ---
    Linh::BytecodeEmitter *emitter;
    if (emitter_ptr)
        emitter = emitter_ptr;
    else
        emitter = new Linh::BytecodeEmitter();
    emitter->emit(ast_statements);
    const auto &chunk = emitter->get_chunk();
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

    // --- Run VM ---
    Linh::LiVM *vm;
    if (vm_ptr)
        vm = vm_ptr;
    else
        vm = new Linh::LiVM();
    vm->run(chunk);

    // Debug: print VM stack and variables after execution (optional)
    // (You can add methods to LiVM to expose stack/vars for debugging if needed)

    std::cout << "\nParse succeeded!" << std::endl;
    if (!sema_ptr)
        delete sema;
    if (!emitter_ptr)
        delete emitter;
    if (!vm_ptr)
        delete vm;
}

void runSource(const std::string &source_code)
{
    runSource(source_code, nullptr, nullptr, nullptr);
}

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        runFile(argv[1]);
    }
    else
    {
        runREPL();
    }

    return 0;
}