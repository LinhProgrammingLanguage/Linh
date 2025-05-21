// main.cpp
#include "Parsing/Lexer/Lexer.hpp"
#include "Parsing/Parser/Parser.hpp" // Include Parser
#include "Parsing/Semantic/SemanticAnalyzer.hpp"
#include "Parsing/tokenizer/TokenTypeUtils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

// Function to print AST (VERY basic example, needs a proper AST Printer/Visitor)
void print_ast_node(const Linh::AST::StmtPtr &stmt_node, int indent = 0); // Forward declare
void print_ast_expr(const Linh::AST::ExprPtr &expr_node, int indent);     // Forward declare

// Basic AST printer implementation (put in a separate file later)
void print_indent(int indent)
{
    for (int i = 0; i < indent; ++i)
        std::cout << "  ";
}

void print_ast_node(const Linh::AST::StmtPtr &stmt_node, int indent)
{
    if (!stmt_node)
    {
        print_indent(indent);
        std::cout << "(Null StmtNode)\n";
        return;
    }
    std::visit([indent](auto &&arg)
               {
        using T = std::decay_t<decltype(arg)>;
        print_indent(indent);
        if constexpr (std::is_same_v<T, Linh::AST::ExpressionStmt>) {
            std::cout << "ExpressionStmt:\n";
            print_ast_expr(arg.expression, indent + 1);
        } else if constexpr (std::is_same_v<T, Linh::AST::PrintStmt>) {
            std::cout << "PrintStmt (token: " << arg.keyword.lexeme << "):\n";
            for(const auto& expr : arg.expressions) {
                print_ast_expr(expr, indent + 1);
            }
        } else if constexpr (std::is_same_v<T, Linh::AST::VarDeclStmt>) {
            std::cout << "VarDeclStmt (" << arg.keyword.lexeme << " " << arg.name.lexeme;
            if(arg.type_annotation) {
                std::cout << ": " << arg.type_annotation->primary_type_token.lexeme;
                // TODO: print complex_type_tokens
            }
            std::cout << "):\n";
            if (arg.initializer) {
                print_indent(indent + 1); std::cout << "Initializer:\n";
                print_ast_expr(*arg.initializer, indent + 2);
            } else {
                print_indent(indent + 1); std::cout << "(No Initializer)\n";
            }
        } else if constexpr (std::is_same_v<T, Linh::AST::BlockStmt>) {
            std::cout << "BlockStmt:\n";
            for (const auto& s : arg.statements) {
                print_ast_node(s, indent + 1);
            }
        } else if constexpr (std::is_same_v<T, Linh::AST::IfStmt>) {
            std::cout << "IfStmt:\n";
            print_indent(indent+1); std::cout << "Condition:\n";
            print_ast_expr(arg.condition, indent + 2);
            print_indent(indent+1); std::cout << "ThenBranch:\n";
            print_ast_node(arg.then_branch, indent + 2);
            if(arg.else_branch) {
                print_indent(indent+1); std::cout << "ElseBranch:\n";
                print_ast_node(*arg.else_branch, indent + 2);
            }
        } else if constexpr (std::is_same_v<T, Linh::AST::WhileStmt>) {
            std::cout << "WhileStmt:\n";
            print_indent(indent+1); std::cout << "Condition:\n";
            print_ast_expr(arg.condition, indent + 2);
            print_indent(indent+1); std::cout << "Body:\n";
            print_ast_node(arg.body, indent + 2);
        } else if constexpr (std::is_same_v<T, Linh::AST::FuncDeclStmt>) {
            std::cout << "FuncDeclStmt (" << arg.name.lexeme << "):\n";
            // TODO: Print params, return type, body
        } else if constexpr (std::is_same_v<T, Linh::AST::ReturnStmt>) {
            std::cout << "ReturnStmt:\n";
            if(arg.value) {
                print_ast_expr(*arg.value, indent + 1);
            } else {
                print_indent(indent+1); std::cout << "(No return value)\n";
            }
        }
         else {
            std::cout << "Unknown StmtNode type\n";
        } }, *stmt_node);
}

void print_ast_expr(const Linh::AST::ExprPtr &expr_node, int indent)
{
    if (!expr_node)
    {
        print_indent(indent);
        std::cout << "(Null ExprNode)\n";
        return;
    }
    std::visit([indent](auto &&arg)
               {
        using T = std::decay_t<decltype(arg)>;
        print_indent(indent);
        if constexpr (std::is_same_v<T, Linh::AST::LiteralExpr>) {
            std::cout << "LiteralExpr: ";
            std::visit([](auto&& val_arg){
                 if constexpr (std::is_same_v<std::decay_t<decltype(val_arg)>, std::string>) std::cout << "\"" << val_arg << "\"";
                 else if constexpr (std::is_same_v<std::decay_t<decltype(val_arg)>, bool>) std::cout << (val_arg ? "true" : "false");
                 else if constexpr (std::is_same_v<std::decay_t<decltype(val_arg)>, std::monostate>) std::cout << "(empty)";
                 else std::cout << val_arg;
            }, arg.value.value);
            std::cout << "\n";
        } else if constexpr (std::is_same_v<T, Linh::AST::UnaryExpr>) {
            std::cout << "UnaryExpr (op: " << arg.op.lexeme << "):\n";
            print_ast_expr(arg.right, indent + 1);
        } else if constexpr (std::is_same_v<T, Linh::AST::BinaryExpr>) {
            std::cout << "BinaryExpr (op: " << arg.op.lexeme << "):\n";
            print_indent(indent + 1); std::cout << "Left:\n";
            print_ast_expr(arg.left, indent + 2);
            print_indent(indent + 1); std::cout << "Right:\n";
            print_ast_expr(arg.right, indent + 2);
        } else if constexpr (std::is_same_v<T, Linh::AST::GroupingExpr>) {
            std::cout << "GroupingExpr:\n";
            print_ast_expr(arg.expression, indent + 1);
        } else if constexpr (std::is_same_v<T, Linh::AST::VariableExpr>) {
            std::cout << "VariableExpr (name: " << arg.name.lexeme << ")\n";
        } else if constexpr (std::is_same_v<T, Linh::AST::AssignExpr>) {
            std::cout << "AssignExpr (name: " << arg.name.lexeme << "):\n";
            print_indent(indent + 1); std::cout << "Value:\n";
            print_ast_expr(arg.value, indent + 2);
        } else if constexpr (std::is_same_v<T, Linh::AST::CallExpr>) {
            std::cout << "CallExpr:\n";
            print_indent(indent+1); std::cout << "Callee:\n";
            print_ast_expr(arg.callee, indent + 2);
            print_indent(indent+1); std::cout << "Arguments:\n";
            for(const auto& an_arg : arg.arguments) {
                print_ast_expr(an_arg, indent + 2);
            }
        }
        else {
            std::cout << "Unknown ExprNode type\n";
        } }, *expr_node);
}

void run_analysis_test(const std::string &source_code, const std::string &description)
{
    std::cout << "\n--- Testing Analyzer: " << description << " ---\n";
    std::cout << "--- Source Code ---\n"
              << source_code << "\n-------------------\n";

    Linh::Lexer lexer(source_code);
    std::vector<Linh::Token> tokens = lexer.scan_tokens();

    // Optional: Print tokens
    // ...

    Linh::Parser parser(tokens);
    Linh::AST::StmtList ast;
    bool parse_had_error = false;
    try
    {
        ast = parser.parse(); // Assume parser might throw on severe errors or sets a flag
                              // Or, change parser.parse() to return bool/optional
    }
    catch (const Linh::ParserError &e)
    {
        std::cout << "Parsing failed with ParserError (already printed).\n";
        parse_had_error = true;
    }
    catch (const std::exception &e)
    {
        std::cout << "Parsing failed with std::exception: " << e.what() << std::endl;
        parse_had_error = true;
    }

    if (parse_had_error || (ast.empty() && !source_code.empty() && source_code.find_first_not_of(" \t\n\r") != std::string::npos))
    { // Check if AST is empty but source code contains non-whitespace characters
        std::cout << "Skipping semantic analysis due to parsing errors or empty AST.\n";
        return;
    }

    std::cout << "--- AST (from Parser) ---\n";
    for (const auto &stmt_node : ast)
    {
        print_ast_node(stmt_node);
    }
    std::cout << "-----------\n";

    Linh::Semantic::SemanticAnalyzer analyzer;
    analyzer.analyze(ast);

    if (analyzer.had_error())
    {
        std::cout << "Semantic analysis failed.\n";
    }
    else
    {
        std::cout << "Semantic analysis completed successfully.\n";
    }
    std::cout << "--------------------------\n";
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    if (argc > 1)
    {
        // ... (file reading logic) ...
    }
    else
    {
        std::cout << "No file provided. Running internal tests.\n";

        // Sử dụng run_analysis_test thay vì run_parser_test
        std::string test_code_semantic1 = R"(
let x: int = 10;
var y = "hello";
print x, y;
// let x = 20; // Error: Redeclaration in same scope (if check is implemented)
)";
        run_analysis_test(test_code_semantic1, "Basic Declarations and Print");

        std::string test_code_semantic2 = R"(
{
    let scoped_var = true;
    print scoped_var;
}
// print scoped_var; // Error: scoped_var not defined here
)";
        run_analysis_test(test_code_semantic2, "Scope Test");

        std::string test_code_semantic3 = R"(
// print undeclared_var; // Error: undeclared
let a = 10;
// a = "text"; // Error: type mismatch for 'let' (if type checking)
const C = 100;
// C = 200; // Error: assignment to const
)";
        run_analysis_test(test_code_semantic3, "Undeclared, Const Assign, Type Mismatch (future)");

        std::string test_code_parser_error = "var err;";
        run_analysis_test(test_code_parser_error, "Parser error propagation test (var decl)"); // Parser should catch this

        std::string test_code_const_ok = "const VAL: int;"; // OK by spec (zero-value)
        run_analysis_test(test_code_const_ok, "Const with type only (OK)");
    }
    return 0;
}