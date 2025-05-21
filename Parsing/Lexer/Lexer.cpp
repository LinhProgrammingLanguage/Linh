#include "Lexer.hpp"
#include <unordered_map>
#include <iostream> // For debugging, can be removed later

namespace Linh
{

    // Static map for keywords
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"var", TokenType::VAR},
        {"let", TokenType::LET},
        {"const", TokenType::CONST},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"for", TokenType::FOR},
        {"while", TokenType::WHILE},
        {"func", TokenType::FUNC},
        {"return", TokenType::RETURN},
        {"true", TokenType::TRUE_KW},
        {"false", TokenType::FALSE_KW},
        {"int", TokenType::INT_TYPE},
        // "int<N>" and "uint<N>" will be IDENTIFIER "int" or "uint" followed by <, NUMBER, >
        // The parser will combine these. Lexer should not try to parse complex type syntax.
        {"uint", TokenType::UINT_TYPE},
        {"str", TokenType::STR_TYPE},
        {"bool", TokenType::BOOL_TYPE},
        {"float", TokenType::FLOAT_TYPE}, // "float<N>" similar to int<N>
        {"map", TokenType::MAP_TYPE},     // "Map<K,V>" similar, Lexer sees IDENTIFIER("Map"), LESS, ...
        {"array", TokenType::ARRAY_TYPE}, // "Type[]" will be IDENTIFIER(Type), LEFT_BRACKET, RIGHT_BRACKET
        {"void", TokenType::VOID_TYPE},
        {"any", TokenType::ANY_TYPE}, // Though "any" cannot be used to declare variables directly
        {"input", TokenType::INPUT},
        {"print", TokenType::PRINT}};

    Lexer::Lexer(std::string source) : m_source(std::move(source)) {}

    bool Lexer::is_at_end() const
    {
        return m_current >= m_source.length();
    }

    char Lexer::advance()
    {
        m_current++;
        return m_source[m_current - 1];
    }

    bool Lexer::match(char expected)
    {
        if (is_at_end())
            return false;
        if (m_source[m_current] != expected)
            return false;
        m_current++;
        return true;
    }

    char Lexer::peek() const
    {
        if (is_at_end())
            return '\0';
        return m_source[m_current];
    }

    char Lexer::peek_next() const
    {
        if (m_current + 1 >= m_source.length())
            return '\0';
        return m_source[m_current + 1];
    }

    int Lexer::current_column() const
    {
        return static_cast<int>(m_start - m_line_start_offset) + 1;
    }

    void Lexer::add_token(TokenType type)
    {
        std::string lexeme = m_source.substr(m_start, m_current - m_start);
        m_tokens.emplace_back(type, std::move(lexeme), m_line, current_column());
    }

    // Removed the overloaded add_token(TokenType, const std::string&) as it was unused and redundant.

    void Lexer::skip_line_comment()
    {
        while (peek() != '\n' && !is_at_end())
        {
            advance();
        }
    }

    void Lexer::skip_block_comment()
    {
        int nesting = 1; // Start with one /*
        while (nesting > 0 && !is_at_end())
        {
            if (peek() == '/' && peek_next() == '*')
            {
                advance(); // /
                advance(); // *
                nesting++;
            }
            else if (peek() == '*' && peek_next() == '/')
            {
                advance(); // *
                advance(); // /
                nesting--;
            }
            else
            {
                if (peek() == '\n')
                {
                    m_line++;
                    m_line_start_offset = m_current + 1; // +1 because advance() will consume the newline
                }
                advance();
            }
        }
        if (nesting > 0)
        {
            // Unterminated block comment
            std::string comment_text = m_source.substr(m_start, m_current - m_start);
            m_tokens.emplace_back(TokenType::UNKNOWN, "Unterminated block comment: " + comment_text, m_line, current_column());
        }
    }

    void Lexer::string_literal(char quote_char)
    {
        // std::string value; // If you were to store the processed string value
        while (peek() != quote_char && !is_at_end())
        {
            if (peek() == '\n' && quote_char != '`')
            {
                // Error: newline in non-template string literal
                m_tokens.emplace_back(TokenType::UNKNOWN, "Newline in non-backtick string literal.", m_line, static_cast<int>(m_current - m_line_start_offset) + 1);
                // For simplicity, let's continue parsing the string.
            }
            if (peek() == '\n')
            { // For all strings, if newline, increment line counter
                m_line++;
                m_line_start_offset = m_current + 1;
            }

            // Basic escape sequence handling for Linh (simplified)
            if (peek() == '\\' && quote_char != '`')
            {              // No escape in raw template literals for now
                advance(); // consume '\'
                if (is_at_end())
                {
                    m_tokens.emplace_back(TokenType::UNKNOWN, "Unterminated string after escape.", m_line, static_cast<int>(m_current - m_line_start_offset) + 1);
                    return;
                }
                // Corrected comment to avoid -Wcomment warning
                // Add more escapes as needed: \t, \r, quote characters, backslash.
                // For now, just take the char after \ literally.
                // Example: if (peek() == 'n') value += '\n'; else if (peek() == '\"') value += '\"'; ...
                // value += peek(); // if building actual value (and handling escapes properly)
            }
            else if (quote_char == '`' && peek() == '&' && peek_next() == '{')
            {
                // For now, treat template string content as regular characters.
                // value += peek(); // if building actual value
            }
            // else {
            // value += peek(); // if building actual value
            // }
            advance();
        }

        if (is_at_end())
        {
            m_tokens.emplace_back(TokenType::UNKNOWN, "Unterminated string.", m_line, current_column());
            return;
        }

        advance(); // Consume the closing quote.
        add_token(TokenType::STRING);
    }

    void Lexer::number_literal()
    {
        while (isdigit(peek()))
        {
            advance();
        }

        TokenType type = TokenType::INTEGER;
        // Look for a fractional part.
        if (peek() == '.' && isdigit(peek_next()))
        {
            type = TokenType::FLOAT;
            advance(); // Consume the "."
            while (isdigit(peek()))
            {
                advance();
            }
        }
        // TODO: Add support for exponents (e.g., 1e10, 3.14e-2) if Linh supports them.
        // TODO: Add support for int<N>, uint<N>, float<N> suffixes if they are part of literal syntax
        // e.g. 10i32, 100u64. For now, type annotations handle this.

        add_token(type);
    }

    void Lexer::identifier_or_keyword()
    {
        while (isalnum(peek()) || peek() == '_')
        {
            advance();
        }

        std::string text = m_source.substr(m_start, m_current - m_start);
        TokenType type;

        auto it = keywords.find(text);
        if (it != keywords.end())
        {
            type = it->second;
        }
        else
        {
            type = TokenType::IDENTIFIER;
        }
        add_token(type);
    }

    void Lexer::scan_token()
    {
        char c = advance();
        switch (c)
        {
        // Single character tokens
        case '(':
            add_token(TokenType::LEFT_PAREN);
            break;
        case ')':
            add_token(TokenType::RIGHT_PAREN);
            break;
        case '{':
            add_token(TokenType::LEFT_BRACE);
            break;
        case '}':
            add_token(TokenType::RIGHT_BRACE);
            break;
        case '[':
            add_token(TokenType::LEFT_BRACKET);
            break;
        case ']':
            add_token(TokenType::RIGHT_BRACKET);
            break;
        case ',':
            add_token(TokenType::COMMA);
            break;
        case ':':
            add_token(TokenType::COLON);
            break;
        case ';':
            add_token(TokenType::SEMICOLON);
            break;
        // Dot is currently not a standalone token unless part of a float.
        // If needed for member access, it would be handled here.
        // case '.': add_token(TokenType::DOT); break;

        // One or two character tokens
        case '!':
            add_token(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
            break;
        case '=':
            add_token(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        case '<':
            add_token(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break; // Parser handles if < is for Union
        case '>':
            add_token(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
            break; // Parser handles if > is for Union
        case '&':
            if (match('&'))
            {
                add_token(TokenType::AND);
            }
            else
            {
                // Nếu chỉ có một '&', theo đặc tả Linh thì nó không phải toán tử hợp lệ
                // Có thể coi là UNKNOWN hoặc nếu bạn muốn hỗ trợ bitwise AND sau này thì là BITWISE_AND
                m_tokens.emplace_back(TokenType::UNKNOWN, "Unexpected character: &", m_line, current_column());
            }
            break;
        case '|':
            if (match('|'))
            {
                add_token(TokenType::OR);
            }
            else
            {
                // Tương tự, nếu chỉ có một '|'
                m_tokens.emplace_back(TokenType::UNKNOWN, "Unexpected character: |", m_line, current_column());
            }
            break;
        case '+':
            add_token(TokenType::PLUS);
            break;
        case '-':
            add_token(TokenType::MINUS);
            break;
        case '*':
            add_token(TokenType::STAR);
            break;
        case '%':
            add_token(TokenType::PERCENT);
            break;

        case '/':
            if (match('/'))
            { // Line comment
                skip_line_comment();
            }
            else if (match('*'))
            { // Block comment
                skip_block_comment();
            }
            else
            {
                add_token(TokenType::SLASH);
            }
            break;

        // Whitespace
        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace.
            break;

        case '\n':
            m_line++;
            m_line_start_offset = m_current; // m_current is now at the start of the new line
            break;

        // String literals
        case '"':
        case '\'':
        case '`': // Backtick for template literals
            string_literal(c);
            break;

        default:
            if (isdigit(c))
            {
                number_literal();
            }
            else if (isalpha(c) || c == '_')
            {
                identifier_or_keyword();
            }
            else
            {
                // Unrecognized character
                std::string lexeme_char;
                lexeme_char += c;
                m_tokens.emplace_back(TokenType::UNKNOWN, "Unexpected character: " + lexeme_char, m_line, current_column());
            }
            break;
        }
    }

    std::vector<Token> Lexer::scan_tokens()
    {
        while (!is_at_end())
        {
            m_start = m_current; // Mark the beginning of the new token
            scan_token();
        }

        m_tokens.emplace_back(TokenType::END_OF_FILE, m_line, static_cast<int>(m_current - m_line_start_offset) + 1);
        return m_tokens;
    }

} // namespace Linh