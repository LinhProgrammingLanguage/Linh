#include "TokenTypeUtils.hpp"
#include <stdexcept> // For std::out_of_range or similar if needed

namespace Linh
{

    // Function definition
    std::string token_type_to_string(Linh::TokenType type)
    {
        switch (type)
        {
        case Linh::TokenType::LEFT_PAREN:
            return "LEFT_PAREN";
        case Linh::TokenType::RIGHT_PAREN:
            return "RIGHT_PAREN";
        case Linh::TokenType::LEFT_BRACE:
            return "LEFT_BRACE";
        case Linh::TokenType::RIGHT_BRACE:
            return "RIGHT_BRACE";
        case Linh::TokenType::LEFT_BRACKET:
            return "LEFT_BRACKET";
        case Linh::TokenType::RIGHT_BRACKET:
            return "RIGHT_BRACKET";
        case Linh::TokenType::COMMA:
            return "COMMA";
        case Linh::TokenType::COLON:
            return "COLON";
        case Linh::TokenType::SEMICOLON:
            return "SEMICOLON";
        case Linh::TokenType::DOT:
            return "DOT";
        case Linh::TokenType::BANG:
            return "BANG";
        case Linh::TokenType::BANG_EQUAL:
            return "BANG_EQUAL";
        case Linh::TokenType::EQUAL:
            return "EQUAL";
        case Linh::TokenType::EQUAL_EQUAL:
            return "EQUAL_EQUAL";
        case Linh::TokenType::GREATER:
            return "GREATER";
        case Linh::TokenType::GREATER_EQUAL:
            return "GREATER_EQUAL";
        case Linh::TokenType::LESS:
            return "LESS";
        case Linh::TokenType::LESS_EQUAL:
            return "LESS_EQUAL";
        case Linh::TokenType::PLUS:
            return "PLUS";
        case Linh::TokenType::MINUS:
            return "MINUS";
        case Linh::TokenType::STAR:
            return "STAR";
        case Linh::TokenType::SLASH:
            return "SLASH";
        case Linh::TokenType::PERCENT:
            return "PERCENT";
        case Linh::TokenType::AND:
            return "AND";
        case Linh::TokenType::OR:
            return "OR";
        case Linh::TokenType::IDENTIFIER:
            return "IDENTIFIER";
        case Linh::TokenType::STRING:
            return "STRING";
        case Linh::TokenType::INTEGER:
            return "INTEGER";
        case Linh::TokenType::FLOAT:
            return "FLOAT";
        case Linh::TokenType::VAR:
            return "VAR";
        case Linh::TokenType::LET:
            return "LET";
        case Linh::TokenType::CONST:
            return "CONST";
        case Linh::TokenType::IF:
            return "IF";
        case Linh::TokenType::ELSE:
            return "ELSE";
        case Linh::TokenType::FOR:
            return "FOR";
        case Linh::TokenType::WHILE:
            return "WHILE";
        case Linh::TokenType::FUNC:
            return "FUNC";
        case Linh::TokenType::RETURN:
            return "RETURN";
        case Linh::TokenType::TRUE_KW:
            return "TRUE_KW";
        case Linh::TokenType::FALSE_KW:
            return "FALSE_KW";
        case Linh::TokenType::INT_TYPE:
            return "INT_TYPE";
        case Linh::TokenType::UINT_TYPE:
            return "UINT_TYPE";
        case Linh::TokenType::STR_TYPE:
            return "STR_TYPE";
        case Linh::TokenType::BOOL_TYPE:
            return "BOOL_TYPE";
        case Linh::TokenType::FLOAT_TYPE:
            return "FLOAT_TYPE";
        case Linh::TokenType::MAP_TYPE:
            return "MAP_TYPE";
        case Linh::TokenType::ARRAY_TYPE:
            return "ARRAY_TYPE";
        case Linh::TokenType::VOID_TYPE:
            return "VOID_TYPE";
        case Linh::TokenType::ANY_TYPE:
            return "ANY_TYPE";
        case Linh::TokenType::INPUT:
            return "INPUT";
        case Linh::TokenType::PRINT:
            return "PRINT";
        case Linh::TokenType::COMMENT:
            return "COMMENT";
        case Linh::TokenType::END_OF_FILE:
            return "END_OF_FILE";
        case Linh::TokenType::UNKNOWN:
            return "UNKNOWN";
        default:
            // This should not happen if all token types are covered
            return "ERROR_UNKNOWN_TOKEN_TYPE_(" + std::to_string(static_cast<int>(type)) + ")";
        }
    }

} // namespace Linh