#ifndef LINH_EXPR_HPP
#define LINH_EXPR_HPP

#include "Parsing/tokenizer/TokenTypeUtils.hpp" // For Token
#include <variant>
#include <vector>
#include <memory> // For std::unique_ptr
#include <string> // For std::string in Literal

namespace Linh
{
    namespace AST
    {

        // Forward declarations for recursive types
        struct BinaryExpr;
        struct UnaryExpr;
        struct LiteralExpr;
        struct GroupingExpr;
        struct VariableExpr;
        struct AssignExpr;
        struct CallExpr;
        // TODO: Add MapLiteralExpr, ArrayLiteralExpr, AccessExpr (for map/array) later

        // Use std::variant to hold different expression types.
        // Each actual expression type will be a struct.
        // We use a unique_ptr to this variant for heap allocation and polymorphism.
        using ExprNode = std::variant<
            struct BinaryExpr,
            struct UnaryExpr,
            struct LiteralExpr,
            struct GroupingExpr,
            struct VariableExpr,
            struct AssignExpr,
            struct CallExpr
            // Add other expression types here
            >;

        // We use unique_ptr for children to manage ownership.
        using ExprPtr = std::unique_ptr<ExprNode>;

        // Helper to create ExprPtr easily
        template <typename T, typename... Args>
        ExprPtr make_expr(Args &&...args)
        {
            return std::make_unique<ExprNode>(T{std::forward<Args>(args)...});
        }

        // ---- Literal Expression ----
        // Represents literals like numbers, strings, true, false
        struct LiteralValue
        { // To hold different types of literal values
            std::variant<std::monostate, long long, double, std::string, bool> value;

            LiteralValue() : value(std::monostate{}) {}
            LiteralValue(long long val) : value(val) {}
            LiteralValue(double val) : value(val) {}
            LiteralValue(std::string val) : value(std::move(val)) {}
            LiteralValue(bool val) : value(val) {}
        };

        struct LiteralExpr
        {
            LiteralValue value;
            // Token literal_token; // Optional: store the original token for error reporting or source mapping

            explicit LiteralExpr(LiteralValue val) : value(std::move(val)) {}
        };

        // ---- Unary Expression ----
        // Represents expressions like !something, -5
        struct UnaryExpr
        {
            Token op; // The operator token (e.g., !, -)
            ExprPtr right;

            UnaryExpr(Token op, ExprPtr right)
                : op(std::move(op)), right(std::move(right)) {}
        };

        // ---- Binary Expression ----
        // Represents expressions like a + b, x == 10
        struct BinaryExpr
        {
            ExprPtr left;
            Token op; // The operator token (e.g., +, ==, &&)
            ExprPtr right;

            BinaryExpr(ExprPtr left, Token op, ExprPtr right)
                : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
        };

        // ---- Grouping Expression ----
        // Represents expressions like (1 + 2)
        struct GroupingExpr
        {
            ExprPtr expression;

            explicit GroupingExpr(ExprPtr expression) : expression(std::move(expression)) {}
        };

        // ---- Variable Expression ----
        // Represents a variable name
        struct VariableExpr
        {
            Token name; // The identifier token

            explicit VariableExpr(Token name) : name(std::move(name)) {}
        };

        // ---- Assignment Expression ----
        // Represents expressions like x = 10, y = "hello"
        struct AssignExpr
        {
            Token name;    // The identifier token for the variable being assigned to
            ExprPtr value; // The value being assigned

            AssignExpr(Token name, ExprPtr value)
                : name(std::move(name)), value(std::move(value)) {}
        };

        // ---- Call Expression ----
        // Represents function calls like print("hello"), add(1, 2)
        struct CallExpr
        {
            ExprPtr callee; // Expression that evaluates to a function (e.g., VariableExpr for 'print')
            Token paren;    // The ')' token, for error reporting location
            std::vector<ExprPtr> arguments;

            CallExpr(ExprPtr callee, Token paren, std::vector<ExprPtr> arguments)
                : callee(std::move(callee)), paren(std::move(paren)), arguments(std::move(arguments)) {}
        };

        // Visitor pattern for expressions (optional for now, but good for later stages like interpreter/compiler)
        // Define later if needed. Example:
        /*
        struct ExprVisitor {
            virtual ~ExprVisitor() = default;
            virtual ReturnType visit_binary_expr(const BinaryExpr& expr) = 0;
            // ... other visit methods
        };
        */

    } // namespace AST
} // namespace Linh

#endif // LINH_EXPR_HPP