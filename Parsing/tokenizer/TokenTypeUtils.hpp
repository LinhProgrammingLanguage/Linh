#ifndef LINH_TOKEN_TYPE_UTILS_HPP
#define LINH_TOKEN_TYPE_UTILS_HPP

#include <string>
#include <vector>  // For Token struct if it uses vector, or if other utils need it
#include <variant> // For Token struct if it uses variant

namespace Linh
{

    // Enum for Token Types
    enum class TokenType
    {
        // Single-character tokens.
        LEFT_PAREN,
        RIGHT_PAREN, // ( )
        LEFT_BRACE,
        RIGHT_BRACE, // { }
        LEFT_BRACKET,
        RIGHT_BRACKET, // [ ]
        COMMA,
        COLON,
        SEMICOLON, // , : ;
        DOT,       // Currently not in spec, but common. Might be needed for float or future features.

        // One or two character tokens.
        BANG,
        BANG_EQUAL, // ! !=
        EQUAL,
        EQUAL_EQUAL, // = ==
        GREATER,
        GREATER_EQUAL, // > >=
        LESS,
        LESS_EQUAL, // < <=
        PLUS,
        MINUS,
        STAR,
        SLASH,
        PERCENT, // + - * / %
        AND,
        OR, // && ||

        // Literals.
        IDENTIFIER,
        STRING,
        INTEGER,
        FLOAT,

        // Keywords. Max 6 chars.
        VAR,
        LET,
        CONST,
        IF,
        ELSE,
        FOR,
        WHILE,
        FUNC,
        RETURN,
        TRUE_KW,
        FALSE_KW,
        INT_TYPE,
        UINT_TYPE,
        STR_TYPE,
        BOOL_TYPE,
        FLOAT_TYPE,
        MAP_TYPE,
        ARRAY_TYPE,
        VOID_TYPE,
        ANY_TYPE,
        INPUT,
        PRINT,

        // Special tokens
        COMMENT, // Will be skipped by the lexer mostly
        END_OF_FILE,
        UNKNOWN // For unrecognized characters/sequences
    };

    // Structure to represent a Token
    struct Token
    {
        TokenType type;
        std::string lexeme;

        int line;
        int column;

        Token(TokenType type, std::string lexeme, int line, int column)
            : type(type), lexeme(std::move(lexeme)), line(line), column(column) {}

        // For simple tokens without a specific lexeme (like EOF)
        Token(TokenType type, int line, int column)
            : type(type), lexeme(""), line(line), column(column) {}
    };

    // Function declaration
    std::string token_type_to_string(Linh::TokenType type);

} // namespace Linh

#endif // LINH_TOKEN_TYPE_UTILS_HPP