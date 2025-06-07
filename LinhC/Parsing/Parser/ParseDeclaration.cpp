// --- LinhC/Parsing/Parser/ParseDeclaration.cpp ---
#include "Parser.hpp"
#include <iostream>
#include <string>

namespace Linh
{
    AST::ExprPtr Parser::create_zero_value_initializer_for_type(const AST::TypeNode *type_node, const Token &reference_token_for_pos)
    {
        if (!type_node)
        {
            Token uninit_token(TokenType::UNINIT_KW, "uninit", std::monostate{}, reference_token_for_pos.line, reference_token_for_pos.column_start);
            return std::make_unique<AST::UninitLiteralExpr>(uninit_token);
        }

        if (const AST::BaseTypeNode *base_type = dynamic_cast<const AST::BaseTypeNode *>(type_node))
        {
            switch (base_type->type_keyword_token.type)
            {
            case TokenType::INT_KW:
            case TokenType::UINT_KW:
            {
                Token zero_int_token(TokenType::INT, "0", static_cast<int64_t>(0), reference_token_for_pos.line, reference_token_for_pos.column_start);
                return std::make_unique<AST::LiteralExpr>(zero_int_token.literal);
            }
            case TokenType::FLOAT_KW:
            {
                Token zero_float_token(TokenType::FLOAT_NUM, "0.0", 0.0, reference_token_for_pos.line, reference_token_for_pos.column_start);
                return std::make_unique<AST::LiteralExpr>(zero_float_token.literal);
            }
            case TokenType::STR_KW:
            {
                Token empty_str_token(TokenType::STR, "\"\"", std::string(""), reference_token_for_pos.line, reference_token_for_pos.column_start);
                return std::make_unique<AST::LiteralExpr>(empty_str_token.literal);
            }
            case TokenType::BOOL_KW:
            {
                Token false_token(TokenType::FALSE_KW, "false", false, reference_token_for_pos.line, reference_token_for_pos.column_start);
                return std::make_unique<AST::LiteralExpr>(false_token.literal);
            }
            case TokenType::UNINIT_KW:
            {
                Token uninit_token(TokenType::UNINIT_KW, "uninit", std::monostate{}, reference_token_for_pos.line, reference_token_for_pos.column_start);
                return std::make_unique<AST::UninitLiteralExpr>(uninit_token);
            }
            case TokenType::VOID_KW:
                throw error(base_type->type_keyword_token, "Cannot create zero value for type 'void'.");
            case TokenType::ANY_KW:
            {
                Token uninit_any_token(TokenType::UNINIT_KW, "uninit", std::monostate{}, reference_token_for_pos.line, reference_token_for_pos.column_start);
                return std::make_unique<AST::UninitLiteralExpr>(uninit_any_token);
            }
            default:
            { // Bao gồm IDENTIFIER
                std::cerr << "PARSER_WARN: Cannot determine zero value for base type '" << base_type->type_keyword_token.lexeme << "' at parse time. Defaulting to uninit." << std::endl;
                Token uninit_default_token(TokenType::UNINIT_KW, "uninit", std::monostate{}, reference_token_for_pos.line, reference_token_for_pos.column_start);
                return std::make_unique<AST::UninitLiteralExpr>(uninit_default_token);
            }
            }
        }
        else if (dynamic_cast<const AST::SizedIntegerTypeNode *>(type_node))
        {
            Token zero_int_token(TokenType::INT, "0", static_cast<int64_t>(0), reference_token_for_pos.line, reference_token_for_pos.column_start);
            return std::make_unique<AST::LiteralExpr>(zero_int_token.literal);
        }
        else if (dynamic_cast<const AST::SizedFloatTypeNode *>(type_node))
        {
            Token zero_float_token(TokenType::FLOAT_NUM, "0.0", 0.0, reference_token_for_pos.line, reference_token_for_pos.column_start);
            return std::make_unique<AST::LiteralExpr>(zero_float_token.literal);
        }
        else if (dynamic_cast<const AST::MapTypeNode *>(type_node))
        {
            Token l_brace(TokenType::LBRACE, "{", std::monostate{}, reference_token_for_pos.line, reference_token_for_pos.column_start);
            Token r_brace(TokenType::RBRACE, "}", std::monostate{}, reference_token_for_pos.line, reference_token_for_pos.column_start);
            return std::make_unique<AST::MapLiteralExpr>(l_brace, std::vector<AST::MapEntryNode>{}, r_brace);
        }
        else if (dynamic_cast<const AST::ArrayTypeNode *>(type_node))
        {
            Token l_bracket(TokenType::LBRACKET, "[", std::monostate{}, reference_token_for_pos.line, reference_token_for_pos.column_start);
            Token r_bracket(TokenType::RBRACKET, "]", std::monostate{}, reference_token_for_pos.line, reference_token_for_pos.column_start);
            return std::make_unique<AST::ArrayLiteralExpr>(l_bracket, std::vector<AST::ExprPtr>{}, r_bracket);
        }
        else if (const AST::UnionTypeNode *union_type = dynamic_cast<const AST::UnionTypeNode *>(type_node))
        {
            if (!union_type->types.empty() && union_type->types[0] != nullptr)
            {
                return create_zero_value_initializer_for_type(union_type->types[0].get(), reference_token_for_pos);
            }
            std::cerr << "PARSER_WARN: Empty union or first type is null when creating zero value. Defaulting to uninit." << std::endl;
        }

        Token uninit_fallback_token(TokenType::UNINIT_KW, "uninit", std::monostate{}, reference_token_for_pos.line, reference_token_for_pos.column_start);
        return std::make_unique<AST::UninitLiteralExpr>(uninit_fallback_token);
    }

    AST::StmtPtr Parser::declaration()
    {
        if (match({TokenType::VAR_KW, TokenType::LET_KW, TokenType::CONST_KW}))
        {
            return var_declaration(previous());
        }
        if (match({TokenType::FUNC_KW}))
        {
            return function_declaration(previous());
        }
        return statement();
    }

    AST::StmtPtr Parser::var_declaration(Token keyword_token)
    {
        Token name_token = consume(TokenType::IDENTIFIER, "Missing variable name after '" + keyword_token.lexeme + "'.");

        std::optional<AST::TypeNodePtr> declared_type_node_opt = std::nullopt;
        if (match({TokenType::COLON}))
        {
            declared_type_node_opt = parse_type();

            if (declared_type_node_opt.has_value() && declared_type_node_opt.value() != nullptr)
            {
                const AST::TypeNode *actual_type_node = declared_type_node_opt.value().get();
                if (const AST::BaseTypeNode *base_type = dynamic_cast<const AST::BaseTypeNode *>(actual_type_node))
                {
                    if (base_type->type_keyword_token.type == TokenType::VOID_KW)
                    {
                        throw error(base_type->type_keyword_token, "Cannot declare variable of type 'void'. 'void' is only allowed as a function return type.");
                    }
                    if (base_type->type_keyword_token.type == TokenType::ANY_KW)
                    {
                        throw error(base_type->type_keyword_token, "Cannot declare variable of type 'any' directly. 'any' should be used as a generic type in map/array or as a return/parameter type when needed.");
                    }
                }
            }
        }

        AST::ExprPtr initializer_expr = nullptr;
        if (match({TokenType::ASSIGN}))
        {
            initializer_expr = expression();
        }
        else
        {
            if (keyword_token.type == TokenType::VAR_KW)
            {
                if (declared_type_node_opt.has_value())
                {
                    initializer_expr = create_zero_value_initializer_for_type(declared_type_node_opt.value().get(), name_token);
                }
                else
                {
                    Token uninit_type_token(TokenType::UNINIT_KW, "uninit", std::monostate{}, name_token.line, name_token.column_start);
                    declared_type_node_opt = std::make_unique<AST::BaseTypeNode>(uninit_type_token);
                    Token uninit_val_token(TokenType::UNINIT_KW, "uninit", std::monostate{}, name_token.line, name_token.column_start);
                    initializer_expr = std::make_unique<AST::UninitLiteralExpr>(uninit_val_token);
                }
            }
            else if (keyword_token.type == TokenType::LET_KW || keyword_token.type == TokenType::CONST_KW)
            {
                if (declared_type_node_opt.has_value())
                {
                    const AST::TypeNode *type_node_ptr = declared_type_node_opt.value().get();
                    if (const AST::BaseTypeNode *base_type = dynamic_cast<const AST::BaseTypeNode *>(type_node_ptr))
                    {
                        if (base_type->type_keyword_token.type == TokenType::UNINIT_KW)
                        {
                            throw error(base_type->type_keyword_token, "'" + keyword_token.lexeme + "' cannot be explicitly declared with type 'uninit'.");
                        }
                    }
                    initializer_expr = create_zero_value_initializer_for_type(type_node_ptr, name_token);
                }
                else
                {
                    throw error(name_token, "'" + keyword_token.lexeme + "' declaration must have an explicit initializer or an explicit type to infer a zero value.");
                }
            }
        }

        if ((keyword_token.type == TokenType::LET_KW || keyword_token.type == TokenType::CONST_KW) &&
            !declared_type_node_opt.has_value() && initializer_expr)
        {
            if (AST::UninitLiteralExpr *uninit_init = dynamic_cast<AST::UninitLiteralExpr *>(initializer_expr.get()))
            {
                Token error_token = uninit_init->keyword;
                throw error(error_token, "Cannot infer fixed type for '" + keyword_token.lexeme + "' from 'uninit' value. Use 'var' or provide an explicit type (e.g., 'let myVar: some_type = uninit;').");
            }
        }

        consume(TokenType::SEMICOLON, "Missing ';' after variable declaration.");
        return std::make_unique<AST::VarDeclStmt>(
            std::move(keyword_token),
            std::move(name_token),
            std::move(declared_type_node_opt),
            std::move(initializer_expr));
    }

    AST::StmtPtr Parser::function_declaration(Token func_keyword)
    {
        Token name_token = consume(TokenType::IDENTIFIER, "Missing function name after 'func'.");
        consume(TokenType::LPAREN, "Missing '(' after function name.");
        std::vector<AST::FuncParamNode> parameters_list;
        if (!check(TokenType::RPAREN))
        {
            do
            {
                if (parameters_list.size() >= 255)
                {
                    error(peek(), "Cannot have more than 255 parameters.");
                }
                Token param_name_token = consume(TokenType::IDENTIFIER, "Missing parameter name.");
                std::optional<AST::TypeNodePtr> param_type_node = std::nullopt;
                if (match({TokenType::COLON}))
                {
                    param_type_node = parse_type();
                    if (param_type_node.has_value() && param_type_node.value())
                    {
                        if (const AST::BaseTypeNode *base_param_type = dynamic_cast<const AST::BaseTypeNode *>(param_type_node.value().get()))
                        {
                            if (base_param_type->type_keyword_token.type == TokenType::VOID_KW)
                            {
                                throw error(base_param_type->type_keyword_token, "Function parameter cannot have type 'void'.");
                            }
                        }
                    }
                }
                parameters_list.emplace_back(param_name_token, std::move(param_type_node));
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RPAREN, "Missing ')' after parameter list.");

        std::optional<AST::TypeNodePtr> return_type_node = std::nullopt;
        if (match({TokenType::COLON}))
        {
            return_type_node = parse_type();
            if (return_type_node.has_value() && return_type_node.value())
            {
                if (const AST::BaseTypeNode *base_ret_type = dynamic_cast<const AST::BaseTypeNode *>(return_type_node.value().get()))
                {
                    if (base_ret_type->type_keyword_token.type == TokenType::UNINIT_KW)
                    {
                        throw error(base_ret_type->type_keyword_token, "Function cannot have return type 'uninit' alone. Use 'void' if no value is returned, or a specific type.");
                    }
                }
            }
        }

        consume(TokenType::LBRACE, "Missing '{' before function body.");
        std::unique_ptr<AST::BlockStmt> body_block_ptr = block();
        return std::make_unique<AST::FunctionDeclStmt>(
            std::move(func_keyword),
            std::move(name_token),
            std::move(parameters_list),
            std::move(return_type_node),
            std::move(body_block_ptr));
    }

} // namespace Linh