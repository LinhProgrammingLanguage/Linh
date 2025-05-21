#include "Parser.hpp"
#include <iostream>  // For error reporting during development
#include <string>    // For std::stoll, std::stod, error messages
#include <stdexcept> // For std::out_of_range, std::invalid_argument
#include <utility>   // For std::move

namespace Linh
{

    // Definition of the template member function parse_binary_helper
    template <typename NextParseFunc, typename... OpTypes>
    AST::ExprPtr Parser::parse_binary_helper(NextParseFunc next_parse_func, OpTypes... op_types)
    {
        AST::ExprPtr expr = (this->*next_parse_func)();
        while (this->match({op_types...}))
        {
            Token op = this->previous();
            AST::ExprPtr right = (this->*next_parse_func)();
            expr = AST::make_expr<AST::BinaryExpr>(std::move(expr), op, std::move(right));
        }
        return expr;
    }

    // Constructor: takes tokens by value and moves them into the member m_tokens
    Parser::Parser(std::vector<Token> tokens_param) : m_tokens(std::move(tokens_param)) {}

    // --- Helper Methods Implementation ---
    bool Parser::is_at_end() const
    {
        // m_tokens should always have at least EOF token if lexer worked correctly.
        if (m_tokens.empty())
        { // Should not happen with a working lexer
            return true;
        }
        return peek().type == TokenType::END_OF_FILE;
    }

    const Token &Parser::peek() const
    {
        if (m_current >= m_tokens.size())
        {
            // This indicates a logical error in parser's advancement.
            // With EOF always present, m_tokens shouldn't be empty.
            // If m_tokens is not empty, return the last token (EOF).
            if (!m_tokens.empty())
            {
                return m_tokens.back();
            }
            // This case is highly problematic and indicates a severe bug.
            throw std::logic_error("Parser::peek() called on an empty or out-of-bounds token stream unexpectedly.");
        }
        return m_tokens[m_current];
    }

    const Token &Parser::previous() const
    {
        if (m_current == 0)
        {
            // This indicates a logic error in parser calling previous() too early.
            throw std::logic_error("Parser::previous() called at the beginning of the token stream.");
        }
        if (m_current > m_tokens.size())
        { // Should not happen
            throw std::logic_error("Parser::previous() called with m_current out of bounds.");
        }
        return m_tokens[m_current - 1];
    }

    Token Parser::advance()
    {
        if (!is_at_end())
        {
            m_current++;
        }
        return previous(); // Returns the token that was just consumed
    }

    bool Parser::check(TokenType type) const
    {
        if (is_at_end())
            return false;
        return peek().type == type;
    }

    bool Parser::check_next(TokenType type) const
    {
        if (is_at_end() || m_current + 1 >= m_tokens.size())
            return false;
        return m_tokens[m_current + 1].type == type;
    }

    bool Parser::match(const std::vector<TokenType> &types)
    {
        for (TokenType type : types)
        {
            if (check(type))
            {
                advance();
                return true;
            }
        }
        return false;
    }

    Token Parser::consume(TokenType type, const std::string &error_message)
    {
        if (check(type))
            return advance();
        throw error(peek(), error_message);
    }

    // --- Error Handling Implementation ---
    ParserError Parser::error(const Token &token, const std::string &message)
    {
        std::cerr << "[Line " << token.line << ", Col " << token.column << "] Error";
        if (token.type == TokenType::END_OF_FILE)
        {
            std::cerr << " at end";
        }
        else
        {
            std::cerr << " at '" << token.lexeme << "'";
        }
        std::cerr << ": " << message << std::endl;
        // Mark that an error occurred if you have a flag for it
        // m_had_error = true;
        return ParserError(token, message);
    }

    void Parser::synchronize()
    {
        advance(); // Consume the token that caused the error

        while (!is_at_end())
        {
            if (previous().type == TokenType::SEMICOLON)
                return; // End of a statement

            switch (peek().type)
            {
            case TokenType::FUNC:
            case TokenType::VAR:
            case TokenType::LET:
            case TokenType::CONST:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::RETURN:
            case TokenType::LEFT_BRACE:
                return;
            default:
                break;
            }
            advance();
        }
    }

    // --- Main Parsing Method ---
    AST::StmtList Parser::parse()
    {
        AST::StmtList statements;
        while (!is_at_end())
        {
            try
            {
                statements.push_back(parse_declaration());
            }
            catch (const ParserError & /*e*/) // Error already printed
            {
                synchronize();
            }
            catch (const std::exception &e) // Catch other potential std exceptions
            {
                std::cerr << "Unhandled std::exception during parsing: " << e.what() << std::endl;
                // Optionally synchronize or rethrow/terminate
                synchronize(); // Attempt to recover
            }
        }
        return statements;
    }

    // --- Parsing Methods for Grammar Rules Implementation ---

    AST::StmtPtr Parser::parse_declaration()
    {
        if (match({TokenType::FUNC}))
            return parse_func_declaration();
        if (match({TokenType::VAR}))
            return parse_var_declaration(TokenType::VAR);
        if (match({TokenType::LET}))
            return parse_var_declaration(TokenType::LET);
        if (match({TokenType::CONST}))
            return parse_var_declaration(TokenType::CONST);

        return parse_statement();
    }

    AST::StmtPtr Parser::parse_var_declaration(TokenType keyword_type)
    {
        Token keyword = previous();
        Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");

        std::optional<AST::TypeAnnotationNode> type_ann;
        if (match({TokenType::COLON}))
        {
            type_ann = parse_type_annotation();
        }

        std::optional<AST::ExprPtr> initializer;
        if (match({TokenType::EQUAL}))
        {
            initializer = parse_expression();
        }

        if (keyword_type == TokenType::CONST && !initializer && !type_ann)
        {
            throw error(name, "Constant declarations must be initialized or have a type for default zero value.");
        }
        // Spec: "const IS_ACTIVATED: bool;" defaults to false. So `!initializer` is okay if `type_ann` exists.
        // The check above handles if BOTH are missing.

        if (keyword_type == TokenType::VAR && !type_ann && !initializer)
        {
            throw error(name, "'var' declarations require a type annotation or an initializer.");
        }

        consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
        return AST::make_stmt<AST::VarDeclStmt>(keyword, name, std::move(type_ann), std::move(initializer));
    }

    AST::StmtPtr Parser::parse_func_declaration()
    {
        Token keyword = previous(); // 'func'
        Token name = consume(TokenType::IDENTIFIER, "Expect function name.");
        consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");

        std::vector<AST::FunctionParameter> params;
        if (!check(TokenType::RIGHT_PAREN))
        {
            do
            {
                if (params.size() >= 255)
                {
                    throw error(peek(), "Cannot have more than 255 parameters.");
                }
                Token param_name = consume(TokenType::IDENTIFIER, "Expect parameter name.");
                consume(TokenType::COLON, "Expect ':' after parameter name for type annotation.");
                AST::TypeAnnotationNode param_type = parse_type_annotation();
                params.emplace_back(param_name, std::move(param_type));
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");

        std::optional<AST::TypeAnnotationNode> return_type;
        if (match({TokenType::COLON}))
        {
            return_type = parse_type_annotation();
        }

        consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
        Token left_brace_for_block = previous(); // Capture the '{' token for BlockStmt

        AST::StmtList body_stmts = parse_block_statement_list();
        AST::StmtPtr body_block = AST::make_stmt<AST::BlockStmt>(left_brace_for_block, std::move(body_stmts));

        return AST::make_stmt<AST::FuncDeclStmt>(keyword, name, std::move(params), std::move(return_type), std::move(body_block));
    }

    AST::StmtPtr Parser::parse_statement()
    {
        if (match({TokenType::PRINT}))
            return parse_print_statement();
        if (match({TokenType::LEFT_BRACE}))
        {
            Token left_brace_token = previous(); // Capture the '{' token
            return AST::make_stmt<AST::BlockStmt>(left_brace_token, parse_block_statement_list());
        }
        if (match({TokenType::IF}))
            return parse_if_statement();
        if (match({TokenType::WHILE}))
            return parse_while_statement();
        if (match({TokenType::FOR})) // TODO: Implement for statement parsing
            throw error(previous(), "For loops are not yet implemented.");
        if (match({TokenType::RETURN}))
            return parse_return_statement();

        return parse_expression_statement();
    }

    AST::StmtList Parser::parse_block_statement_list()
    {
        AST::StmtList statements;
        while (!check(TokenType::RIGHT_BRACE) && !is_at_end())
        {
            statements.push_back(parse_declaration());
        }
        consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
        return statements;
    }

    AST::StmtPtr Parser::parse_print_statement()
    {
        Token keyword = previous();
        std::vector<AST::ExprPtr> expressions;
        if (!check(TokenType::SEMICOLON)) // Check if there are arguments before semicolon
        {
            do
            {
                expressions.push_back(parse_expression());
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::SEMICOLON, "Expect ';' after print statement arguments.");
        return AST::make_stmt<AST::PrintStmt>(keyword, std::move(expressions));
    }

    AST::StmtPtr Parser::parse_if_statement()
    {
        Token keyword = previous();
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
        AST::ExprPtr condition = parse_expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");

        AST::StmtPtr then_branch = parse_statement();
        std::optional<AST::StmtPtr> else_branch = std::nullopt;
        if (match({TokenType::ELSE}))
        {
            else_branch = parse_statement();
        }
        return AST::make_stmt<AST::IfStmt>(keyword, std::move(condition), std::move(then_branch), std::move(else_branch));
    }

    AST::StmtPtr Parser::parse_while_statement()
    {
        Token keyword = previous();
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
        AST::ExprPtr condition = parse_expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after while condition.");
        AST::StmtPtr body = parse_statement();
        return AST::make_stmt<AST::WhileStmt>(keyword, std::move(condition), std::move(body));
    }

    AST::StmtPtr Parser::parse_for_statement()
    {
        // Placeholder for FOR loop parsing - currently throws error in parse_statement
        throw error(previous(), "For loops are not yet implemented in parse_for_statement.");
    }

    AST::StmtPtr Parser::parse_return_statement()
    {
        Token keyword = previous();
        std::optional<AST::ExprPtr> value = std::nullopt;
        if (!check(TokenType::SEMICOLON))
        {
            value = parse_expression();
        }
        consume(TokenType::SEMICOLON, "Expect ';' after return value.");
        return AST::make_stmt<AST::ReturnStmt>(keyword, std::move(value));
    }

    AST::StmtPtr Parser::parse_expression_statement()
    {
        AST::ExprPtr expr = parse_expression();
        consume(TokenType::SEMICOLON, "Expect ';' after expression.");
        return AST::make_stmt<AST::ExpressionStmt>(std::move(expr));
    }

    // --- Expression Parsing (Pratt Parser style or recursive descent with precedence) ---
    AST::ExprPtr Parser::parse_expression()
    {
        return parse_assignment();
    }

    AST::ExprPtr Parser::parse_assignment()
    {
        AST::ExprPtr expr = parse_logical_or();
        if (match({TokenType::EQUAL}))
        {
            Token equals = previous();
            AST::ExprPtr value = parse_assignment(); // Right-associative
            if (auto *var_expr_node = std::get_if<AST::VariableExpr>(&(*expr)))
            {
                return AST::make_expr<AST::AssignExpr>(var_expr_node->name, std::move(value));
            }
            // TODO: Add support for assigning to map/array elements (l-values)
            throw error(equals, "Invalid assignment target.");
        }
        return expr;
    }

    AST::ExprPtr Parser::parse_logical_or()
    {
        return parse_binary_helper(&Parser::parse_logical_and, TokenType::OR);
    }

    AST::ExprPtr Parser::parse_logical_and()
    {
        return parse_binary_helper(&Parser::parse_equality, TokenType::AND);
    }

    AST::ExprPtr Parser::parse_equality()
    {
        return parse_binary_helper(&Parser::parse_comparison, TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL);
    }

    AST::ExprPtr Parser::parse_comparison()
    {
        return parse_binary_helper(&Parser::parse_term, TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL);
    }

    AST::ExprPtr Parser::parse_term()
    {
        return parse_binary_helper(&Parser::parse_factor, TokenType::MINUS, TokenType::PLUS);
    }

    AST::ExprPtr Parser::parse_factor()
    {
        return parse_binary_helper(&Parser::parse_unary, TokenType::SLASH, TokenType::STAR, TokenType::PERCENT);
    }

    AST::ExprPtr Parser::parse_unary()
    {
        if (match({TokenType::BANG, TokenType::MINUS}))
        {
            Token op = previous();
            AST::ExprPtr right = parse_unary();
            return AST::make_expr<AST::UnaryExpr>(op, std::move(right));
        }
        return parse_call();
    }

    AST::ExprPtr Parser::parse_call()
    {
        AST::ExprPtr expr = parse_primary();
        while (true)
        {
            if (match({TokenType::LEFT_PAREN}))
            {
                expr = finish_call(std::move(expr));
            }
            // TODO: Add LEFT_BRACKET for array/map access `expr[index]`
            // TODO: Add DOT for member access `expr.member`
            else
            {
                break;
            }
        }
        return expr;
    }

    AST::ExprPtr Parser::finish_call(AST::ExprPtr callee)
    {
        std::vector<AST::ExprPtr> arguments;
        if (!check(TokenType::RIGHT_PAREN))
        {
            do
            {
                if (arguments.size() >= 255)
                {
                    // Report error but don't throw immediately to try parsing rest of args.
                    // ParserError will be caught by main parse loop.
                    error(peek(), "Cannot have more than 255 arguments.");
                    // To stop adding arguments after this error, you could break or return,
                    // but parsing the rest might reveal more syntax errors.
                }
                arguments.push_back(parse_expression());
            } while (match({TokenType::COMMA}));
        }
        Token paren = consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
        return AST::make_expr<AST::CallExpr>(std::move(callee), paren, std::move(arguments));
    }

    AST::ExprPtr Parser::parse_primary()
    {
        if (match({TokenType::FALSE_KW}))
            return AST::make_expr<AST::LiteralExpr>(false);
        if (match({TokenType::TRUE_KW}))
            return AST::make_expr<AST::LiteralExpr>(true);

        if (match({TokenType::INTEGER}))
        {
            try
            {
                return AST::make_expr<AST::LiteralExpr>(std::stoll(previous().lexeme));
            }
            catch (const std::out_of_range &)
            {
                throw error(previous(), "Integer literal out of range.");
            }
            catch (const std::invalid_argument &)
            {
                throw error(previous(), "Invalid integer literal format.");
            }
        }
        if (match({TokenType::FLOAT}))
        {
            try
            {
                return AST::make_expr<AST::LiteralExpr>(std::stod(previous().lexeme));
            }
            catch (const std::out_of_range &)
            {
                throw error(previous(), "Float literal out of range.");
            }
            catch (const std::invalid_argument &)
            {
                throw error(previous(), "Invalid float literal format.");
            }
        }
        if (match({TokenType::STRING}))
        {
            std::string str_val = previous().lexeme;
            // Unquote the string
            if (str_val.length() >= 2 &&
                ((str_val.front() == '"' && str_val.back() == '"') ||
                 (str_val.front() == '\'' && str_val.back() == '\'') ||
                 (str_val.front() == '`' && str_val.back() == '`')))
            {
                return AST::make_expr<AST::LiteralExpr>(str_val.substr(1, str_val.length() - 2));
            }
            // Should not happen if lexer is correct, but as a fallback:
            return AST::make_expr<AST::LiteralExpr>(str_val);
        }

        if (match({TokenType::IDENTIFIER}))
        {
            return AST::make_expr<AST::VariableExpr>(previous());
        }

        if (match({TokenType::LEFT_PAREN}))
        {
            AST::ExprPtr expr = parse_expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
            return AST::make_expr<AST::GroupingExpr>(std::move(expr));
        }

        // TODO: Parse Map literals like `{"key": value}`
        // TODO: Parse Array literals like `[1, "two"]`

        throw error(peek(), "Expect expression.");
    }

    // --- Type Annotation Parsing ---
    AST::TypeAnnotationNode Parser::parse_type_annotation()
    {
        // Case 1: Simple type name or keyword
        if (check(TokenType::INT_TYPE) || check(TokenType::STR_TYPE) ||
            check(TokenType::BOOL_TYPE) || check(TokenType::FLOAT_TYPE) ||
            check(TokenType::UINT_TYPE) || check(TokenType::VOID_TYPE) ||
            check(TokenType::MAP_TYPE) || check(TokenType::ARRAY_TYPE) ||
            check(TokenType::ANY_TYPE) ||
            check(TokenType::IDENTIFIER))
        {
            Token primary_token = advance();
            AST::TypeAnnotationNode type_node(std::move(primary_token));

            // Handle `Type[]`
            if (match({TokenType::LEFT_BRACKET}))
            {
                type_node.complex_type_tokens.push_back(previous());                                                      // '['
                type_node.complex_type_tokens.push_back(consume(TokenType::RIGHT_BRACKET, "Expect ']' for array type.")); // ']'
            }
            // Handle generics like `Map<K,V>` or `int<N>`
            else if (check(TokenType::LESS)) // Must be LESS, not match, as we need to parse content
            {
                type_node.complex_type_tokens.push_back(advance()); // Consume '<'
                int balance = 1;
                while (balance > 0 && !is_at_end())
                {
                    if (peek().type == TokenType::LESS)
                        balance++;
                    else if (peek().type == TokenType::GREATER)
                        balance--;

                    // Check for tokens that clearly end a type annotation in common contexts
                    // to prevent consuming too much on error.
                    if (balance > 0 && // Only break if we are not expecting a '>'
                        (peek().type == TokenType::EQUAL || peek().type == TokenType::SEMICOLON ||
                         peek().type == TokenType::RIGHT_PAREN || peek().type == TokenType::LEFT_BRACE))
                    {
                        throw error(peek(), "Unbalanced '<' in type annotation, unexpected token found.");
                    }

                    type_node.complex_type_tokens.push_back(advance()); // Consume content token

                    if (type_node.complex_type_tokens.size() > 50)
                    { // Safeguard
                        throw error(type_node.primary_type_token, "Type annotation seems excessively complex or malformed.");
                    }

                    if (balance == 0 && type_node.complex_type_tokens.back().type == TokenType::GREATER)
                    {
                        break; // Successfully closed the generic/union part
                    }
                }
                if (balance > 0)
                {
                    throw error(type_node.complex_type_tokens.empty() ? type_node.primary_type_token : type_node.complex_type_tokens.back(),
                                "Unterminated '<' in type annotation.");
                }
            }
            return type_node;
        }
        // Case 2: Union type starting directly with `<`
        else if (match({TokenType::LESS})) // Consumes the '<'
        {
            Token primary_token = previous(); // This is the '<' token
            AST::TypeAnnotationNode type_node(std::move(primary_token));
            // No need to add primary_token to complex_type_tokens again, it's the main identifier of this union type.
            // complex_type_tokens will hold the types *inside* the union.

            int balance = 1; // Already consumed one '<' via match()
            while (balance > 0 && !is_at_end())
            {
                if (peek().type == TokenType::LESS)
                    balance++;
                else if (peek().type == TokenType::GREATER)
                    balance--;

                if (balance > 0 &&
                    (peek().type == TokenType::EQUAL || peek().type == TokenType::SEMICOLON ||
                     peek().type == TokenType::RIGHT_PAREN || peek().type == TokenType::LEFT_BRACE))
                {
                    throw error(peek(), "Unbalanced '<' in union type annotation, unexpected token found.");
                }

                type_node.complex_type_tokens.push_back(advance());

                if (type_node.complex_type_tokens.size() > 50)
                {
                    throw error(type_node.primary_type_token, "Union type annotation seems excessively complex or malformed.");
                }

                if (balance == 0 && type_node.complex_type_tokens.back().type == TokenType::GREATER)
                {
                    break;
                }
            }
            if (balance > 0)
            {
                throw error(type_node.complex_type_tokens.empty() ? type_node.primary_type_token : type_node.complex_type_tokens.back(),
                            "Unterminated '<' in union type annotation.");
            }
            return type_node;
        }
        else
        {
            throw error(peek(), "Expect type name or '<' for union type after ':'.");
        }
    }

    std::optional<AST::TypeAnnotationNode> Parser::parse_type_annotation_if_present()
    {
        if (match({TokenType::COLON}))
        {
            return parse_type_annotation();
        }
        return std::nullopt;
    }

} // namespace Linh