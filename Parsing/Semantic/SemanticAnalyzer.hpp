#ifndef LINH_SEMANTIC_ANALYZER_HPP
#define LINH_SEMANTIC_ANALYZER_HPP

#include "Parsing/AST/Stmt.hpp"
#include "Parsing/AST/Expr.hpp"
#include "Parsing/tokenizer/TokenTypeUtils.hpp" // For Token
#include <vector>
#include <string>
#include <memory>
#include <map>       // For symbol table
#include <stdexcept> // For semantic errors

namespace Linh
{
    namespace Semantic
    {

        // Simple structure to hold information about a declared symbol
        struct SymbolInfo
        {
            Token declaration_token; // Token where it was declared (for error messages)
            // TODO: AST::Type resolved_type; // Store resolved type information
            bool is_initialized = false;
            bool is_constant = false;
            // Add more info as needed: is_used, scope_level, etc.

            // Constructor
            SymbolInfo(Token token, bool constant = false, bool initialized = false)
                : declaration_token(std::move(token)), is_initialized(initialized), is_constant(constant) {}
        };

        // Environment to manage scopes and symbols
        class Environment
        {
        public:
            Environment();

            // Defines a new variable in the current scope.
            // Throws std::runtime_error on redeclaration in the same scope.
            void define(const std::string &name, const Token &declaration_token, bool is_constant, bool is_initialized);

            // Looks up a variable name in the environment, searching from current scope outwards.
            // Returns a const reference to SymbolInfo.
            // Throws std::runtime_error if the variable is not found.
            const SymbolInfo &get(const Token &name_token) const;

            // Assigns to an existing variable.
            // Checks for const violations and marks the variable as initialized.
            // Throws std::runtime_error if variable not found or assignment to const.
            void assign(const Token &name_token, bool is_reassigning_with_value = true);

            void begin_scope();
            void end_scope();

        private:
            // Using std::vector of maps for lexical scoping.
            // Each map represents a scope: string (name) -> SymbolInfo
            // m_scopes.back() is the current innermost scope.
            std::vector<std::map<std::string, SymbolInfo>> m_scopes;

            // Helper to find a symbol walking up the scope chain (const version).
            // Returns a const pointer to the symbol info, or nullptr if not found.
            const SymbolInfo *find_symbol(const std::string &name) const;

            // Helper to find a symbol walking up the scope chain (modifiable version).
            // Returns a pointer to the symbol info, or nullptr if not found.
            SymbolInfo *find_symbol_modifiable(const std::string &name);
        };

        // Semantic Analyzer class using Visitor pattern (simulated with std::visit)
        class SemanticAnalyzer
        {
        public:
            SemanticAnalyzer();

            // Analyzes a list of statements.
            // Sets had_error() flag if any semantic errors are found.
            // Throws std::runtime_error on the first semantic error to halt analysis.
            void analyze(const AST::StmtList &statements);

            bool had_error() const { return m_had_error; }

        private:
            Environment m_environment;
            bool m_had_error = false;
            // Token m_current_function_token; // TODO: To check 'return' statements validity

            // --- Error Reporting ---
            // Reports a semantic error and throws std::runtime_error to stop analysis.
            void error(const Token &token, const std::string &message);

            // --- Visitor Methods (simulated with std::visit) ---
            // Statements
            void visit_stmt(const AST::StmtPtr &stmt); // Dispatcher
            void visit_expression_stmt(const AST::ExpressionStmt &stmt);
            void visit_print_stmt(const AST::PrintStmt &stmt);
            void visit_var_decl_stmt(const AST::VarDeclStmt &stmt);
            void visit_block_stmt(const AST::BlockStmt &stmt);
            void visit_if_stmt(const AST::IfStmt &stmt);
            void visit_while_stmt(const AST::WhileStmt &stmt);
            void visit_func_decl_stmt(const AST::FuncDeclStmt &stmt);
            void visit_return_stmt(const AST::ReturnStmt &stmt);

            // Expressions
            void visit_expr(const AST::ExprPtr &expr); // Dispatcher
            void visit_literal_expr(const AST::LiteralExpr &expr);
            void visit_unary_expr(const AST::UnaryExpr &expr);
            void visit_binary_expr(const AST::BinaryExpr &expr);
            void visit_grouping_expr(const AST::GroupingExpr &expr);
            void visit_variable_expr(const AST::VariableExpr &expr);
            void visit_assign_expr(const AST::AssignExpr &expr);
            void visit_call_expr(const AST::CallExpr &expr);

            // Helper to resolve/analyze an expression
            void resolve_expression(const AST::ExprPtr &expr);
            // Helper to resolve/analyze a statement
            void resolve_statement(const AST::StmtPtr &stmt);
        };

    } // namespace Semantic
} // namespace Linh

#endif // LINH_SEMANTIC_ANALYZER_HPP