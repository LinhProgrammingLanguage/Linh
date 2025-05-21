#ifndef LINH_LEXER_HPP
#define LINH_LEXER_HPP

#include <string>
#include <vector>
// #include <variant> // No longer needed here, moved to TokenTypeUtils.hpp if Token uses it

// Include the new header for TokenType and Token definitions
#include "Parsing/tokenizer/TokenTypeUtils.hpp" // Adjusted path

namespace Linh
{

    // TokenType and Token are now defined in TokenTypeUtils.hpp

    class Lexer
    {
    public:
        explicit Lexer(std::string source);
        std::vector<Token> scan_tokens();

    private:
        const std::string m_source;
        std::vector<Token> m_tokens;

        size_t m_start = 0;
        size_t m_current = 0;
        int m_line = 1;
        int m_line_start_offset = 0;

        bool is_at_end() const;
        char advance();
        bool match(char expected);
        char peek() const;
        char peek_next() const;

        void scan_token();

        void add_token(TokenType type);

        void string_literal(char quote_char);
        void number_literal();
        void identifier_or_keyword();
        void skip_block_comment();
        void skip_line_comment();

        int current_column() const;
    };

} // namespace Linh

#endif // LINH_LEXER_HPP