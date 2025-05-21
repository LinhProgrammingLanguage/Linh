// Parsing/AST/Stmt.hpp
#ifndef LINH_STMT_HPP
#define LINH_STMT_HPP

#include "Expr.hpp"                             // Needs ExprPtr
#include "Parsing/tokenizer/TokenTypeUtils.hpp" // For Token
#include <variant>
#include <vector>
#include <memory>   // For std::unique_ptr
#include <optional> // For optional parts like initializer or type annotation
#include <string>   // For TypeAnnotationNode/Token

namespace Linh
{
    namespace AST
    {

        // Forward declarations
        struct ExpressionStmt;
        struct PrintStmt; // Example for built-in print
        struct VarDeclStmt;
        struct BlockStmt;
        struct IfStmt;
        struct WhileStmt;
        struct FuncDeclStmt;
        struct ReturnStmt;
        // TODO: ForStmt

        // Use std::variant to hold different statement types.
        using StmtNode = std::variant<
            struct ExpressionStmt,
            struct PrintStmt,
            struct VarDeclStmt,
            struct BlockStmt,
            struct IfStmt,
            struct WhileStmt,
            struct FuncDeclStmt,
            struct ReturnStmt
            // Add other statement types here
            >;

        using StmtPtr = std::unique_ptr<StmtNode>;
        using StmtList = std::vector<StmtPtr>;

        // Helper to create StmtPtr easily
        template <typename T, typename... Args>
        StmtPtr make_stmt(Args &&...args)
        {
            return std::make_unique<StmtNode>(T{std::forward<Args>(args)...});
        }

        // ---- Type Annotation Node ----
        // Represents a type annotation like `: int`, `: str`, `: <str, int>`
        struct TypeAnnotationNode
        {
            // Stores the primary type token (e.g., IDENTIFIER("int"), INT_TYPE, MAP_TYPE, LESS for union)
            Token primary_type_token;
            // Stores additional tokens for complex types (e.g., `<`, `str`, `,`, `int`, `>`, `[`, `]`)
            std::vector<Token> complex_type_tokens;

            // Constructor to initialize with the primary token
            explicit TypeAnnotationNode(Token primary_token) : primary_type_token(std::move(primary_token)) {}
        };

        // ---- Expression Statement ----
        // A statement that is just an expression (e.g., function call, assignment)
        struct ExpressionStmt
        {
            ExprPtr expression;

            explicit ExpressionStmt(ExprPtr expression) : expression(std::move(expression)) {}
        };

        // ---- Print Statement (Example for built-in) ----
        // If `print` is treated as a special statement form
        struct PrintStmt
        {
            Token keyword; // The 'print' token, for location
            std::vector<ExprPtr> expressions;

            // Constructor - Ensure initializer list order matches member declaration order
            PrintStmt(Token keyword_token, std::vector<ExprPtr> expr_list)
                : keyword(std::move(keyword_token)), expressions(std::move(expr_list)) {}
        };

        // ---- Variable Declaration Statement ----
        // Covers 'var', 'let', 'const'
        struct VarDeclStmt
        {
            Token keyword; // var, let, or const
            Token name;
            std::optional<TypeAnnotationNode> type_annotation; // Optional type: e.g. `: int`
            std::optional<ExprPtr> initializer;                // Optional initializer: e.g. `= 10`

            VarDeclStmt(Token keyword_token, Token name_token,
                        std::optional<TypeAnnotationNode> type,
                        std::optional<ExprPtr> initializer_expr)
                : keyword(std::move(keyword_token)),
                  name(std::move(name_token)),
                  type_annotation(std::move(type)),
                  initializer(std::move(initializer_expr)) {}
        };

        // ---- Block Statement ----
        // A block of statements: { ... }
        struct BlockStmt
        {
            Token left_brace; // For location/debugging
            StmtList statements;

            // Constructor - Ensure initializer list order matches member declaration order
            BlockStmt(Token left_brace_token, StmtList stmt_list)
                : left_brace(std::move(left_brace_token)), statements(std::move(stmt_list)) {}
        };

        // ---- If Statement ----
        struct IfStmt
        {
            Token keyword; // The 'if' token
            ExprPtr condition;
            StmtPtr then_branch;
            std::optional<StmtPtr> else_branch; // Optional else

            IfStmt(Token keyword_token, ExprPtr condition_expr, StmtPtr then_stmt, std::optional<StmtPtr> else_stmt)
                : keyword(std::move(keyword_token)),
                  condition(std::move(condition_expr)),
                  then_branch(std::move(then_stmt)),
                  else_branch(std::move(else_stmt)) {}
        };

        // ---- While Statement ----
        struct WhileStmt
        {
            Token keyword; // The 'while' token
            ExprPtr condition;
            StmtPtr body;

            WhileStmt(Token keyword_token, ExprPtr condition_expr, StmtPtr body_stmt)
                : keyword(std::move(keyword_token)),
                  condition(std::move(condition_expr)),
                  body(std::move(body_stmt)) {}
        };

        // ---- Function Parameter ----
        struct FunctionParameter
        {
            Token name;
            TypeAnnotationNode type_annotation; // Parameters in Linh seem to require types

            FunctionParameter(Token name_token, TypeAnnotationNode type)
                : name(std::move(name_token)), type_annotation(std::move(type)) {}
        };

        // ---- Function Declaration Statement ----
        struct FuncDeclStmt
        {
            Token keyword; // The 'func' token
            Token name;
            std::vector<FunctionParameter> params;
            std::optional<TypeAnnotationNode> return_type; // `void` or other types
            // Body is a block statement.
            StmtPtr body; // Should point to an AST::BlockStmt

            FuncDeclStmt(Token keyword_token, Token name_token, std::vector<FunctionParameter> param_list,
                         std::optional<TypeAnnotationNode> return_type_ann, StmtPtr body_block)
                : keyword(std::move(keyword_token)),
                  name(std::move(name_token)),
                  params(std::move(param_list)),
                  return_type(std::move(return_type_ann)),
                  body(std::move(body_block)) {}
        };

        // ---- Return Statement ----
        struct ReturnStmt
        {
            Token keyword;                // The 'return' token
            std::optional<ExprPtr> value; // Optional value to return

            ReturnStmt(Token keyword_token, std::optional<ExprPtr> return_value)
                : keyword(std::move(keyword_token)), value(std::move(return_value)) {}
        };

        // Visitor pattern for statements (optional for now)
        /*
        struct StmtVisitor {
            virtual ~StmtVisitor() = default;
            virtual ReturnType visit_expression_stmt(const ExpressionStmt& stmt) = 0;
            // ... other visit methods
        };
        */

    } // namespace AST
} // namespace Linh

#endif // LINH_STMT_HPP