#ifndef LINH_PARSER_HPP
#define LINH_PARSER_HPP

#include "Parsing/tokenizer/TokenTypeUtils.hpp" // For Token and TokenType
#include "Parsing/AST/Expr.hpp"                 // For ExprPtr
#include "Parsing/AST/Stmt.hpp"                 // For StmtPtr, StmtList
#include <vector>
#include <stdexcept> // For runtime_error
#include <optional>  // For std::optional used in AST nodes
#include <string>    // For error messages

namespace Linh
{

    class ParserError : public std::runtime_error
    {
    public:
        Token token; // Token that caused the error
        ParserError(const Token &token, const std::string &message)
            : std::runtime_error(message), token(token) {}
    };

    class Parser
    {
    public:
        // Constructor now takes tokens by value, and m_tokens will own them
        explicit Parser(std::vector<Token> tokens);

        // Main parsing method
        AST::StmtList parse();

    private:
        // m_tokens now holds a copy of the tokens, not a reference
        const std::vector<Token> m_tokens;
        size_t m_current = 0; // Index of the next token to consume

        // --- Helper Methods ---
        bool is_at_end() const;
        const Token &peek() const;                       // Returns current token without consuming
        const Token &previous() const;                   // Returns the last consumed token
        Token advance();                                 // Consumes and returns current token
        bool check(TokenType type) const;                // Checks if current token is of a given type
        bool check_next(TokenType type) const;           // Checks if the next token is of a given type
        bool match(const std::vector<TokenType> &types); // Consumes if current token matches any of types
        Token consume(TokenType type, const std::string &error_message);

        // --- Error Handling ---
        ParserError error(const Token &token, const std::string &message);
        void synchronize(); // For error recovery

        // --- Parsing Methods for Grammar Rules ---
        // Declarations
        AST::StmtPtr parse_declaration();
        AST::StmtPtr parse_var_declaration(TokenType keyword_type); // For var, let, const
        AST::StmtPtr parse_func_declaration();

        // Statements
        AST::StmtPtr parse_statement();
        AST::StmtPtr parse_print_statement(); // If 'print' is a special statement
        AST::StmtPtr parse_expression_statement();
        AST::StmtList parse_block_statement_list(); // Parses statements inside a block
        AST::StmtPtr parse_if_statement();
        AST::StmtPtr parse_while_statement();
        AST::StmtPtr parse_for_statement(); // TODO: Implement if needed
        AST::StmtPtr parse_return_statement();

        // Expressions (following precedence rules)
        AST::ExprPtr parse_expression();
        AST::ExprPtr parse_assignment();
        AST::ExprPtr parse_logical_or();  // Precedence: ||
        AST::ExprPtr parse_logical_and(); // Precedence: &&
        AST::ExprPtr parse_equality();    // Precedence: == !=
        AST::ExprPtr parse_comparison();  // Precedence: > >= < <=
        AST::ExprPtr parse_term();        // Precedence: + -
        AST::ExprPtr parse_factor();      // Precedence: * / %
        AST::ExprPtr parse_unary();       // Precedence: ! -
        AST::ExprPtr parse_call();        // Function calls, member access (if any)
        AST::ExprPtr parse_primary();     // Literals, identifiers, grouping

        // Helper for call expressions
        AST::ExprPtr finish_call(AST::ExprPtr callee);

        // Helper for type annotations
        std::optional<AST::TypeAnnotationNode> parse_type_annotation_if_present();
        AST::TypeAnnotationNode parse_type_annotation(); // Assumes colon has been consumed

        // Helper for binary expressions
        template <typename NextParseFunc, typename... OpTypes>
        AST::ExprPtr parse_binary_helper(NextParseFunc next_parse_func, OpTypes... op_types);
    };

} // namespace Linh

#endif // LINH_PARSER_HPP