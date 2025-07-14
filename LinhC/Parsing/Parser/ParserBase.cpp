// --- LinhC/Parsing/Parser/ParserBase.cpp ---
#include "Parser.hpp" // Header chính của Parser
#include <iostream>   // Cho std::cout, std::cerr
#include <string>     // Cho std::string, std::to_string
#include <algorithm>  // Có thể cần cho một số tiện ích sau này

// Helper để in token cho debug (có thể giữ ở đây hoặc chuyển vào một tệp tiện ích chung)
#ifdef _DEBUG
static std::string debug_token_info(const Linh::Token &t)
{
    if (t.type == Linh::TokenType::END_OF_FILE && t.lexeme.empty())
    {
        return "'EOF' (Type: " + Linh::token_type_to_string(t.type) +
               ", Line: " + std::to_string(t.line) +
               ", Col: " + std::to_string(t.column_start) + ")";
    }
    return "'" + t.lexeme + "' (Type: " + Linh::token_type_to_string(t.type) +
           ", Line: " + std::to_string(t.line) +
           ", Col: " + std::to_string(t.column_start) + ")";
}
#endif

namespace Linh
{
    // --- Constructor ---
    Parser::Parser(const std::vector<Token> &tokens) : m_tokens(tokens), m_current(0), m_had_error(false)
    {
#ifdef _DEBUG
        std::cout << "PARSER_INIT: Parser initialized with " << tokens.size() << " tokens.\n";
#endif
        if (tokens.empty() || tokens.back().type != TokenType::END_OF_FILE)
        {
            std::cerr << "WARNING [Line: 0, Col: 0] ParserWarning: Token list is invalid or missing EOF." << std::endl;
        }
        else if (!tokens.empty())
        {
#ifdef _DEBUG
            std::cout << "PARSER_INIT: First token: " << debug_token_info(m_tokens[0]) << std::endl;
            std::cout << "PARSER_INIT: Last token: " << debug_token_info(m_tokens.back()) << std::endl;
#endif
        }
    }

    // --- Utility Methods ---
    Token Parser::previous() const
    {
        if (m_current == 0)
        {
            // Đây là một lỗi logic trong chính Parser nếu nó cố gắng gọi previous()
            // trước khi có bất kỳ advance() nào, hoặc khi danh sách token rỗng và m_current = 0.
            // std::cerr << "PARSER_UTIL_ERROR: previous() called when m_current is 0." << std::endl;
            throw std::logic_error("Parser::previous() called at the beginning of tokens or with empty token list.");
        }
        return m_tokens[m_current - 1];
    }

    Token Parser::peek() const
    {
        if (m_tokens.empty())
        {
            // std::cerr << "PARSER_UTIL_ERROR: peek() called on empty token list." << std::endl;
            throw std::logic_error("Parser::peek() called on empty token list.");
        }
        // is_at_end() sẽ đảm bảo m_current không vượt quá giới hạn khi trả về EOF
        // Nếu m_current >= m_tokens.size(), nó sẽ bị bắt bởi is_at_end() trước khi peek() được gọi trong logic chính.
        // Tuy nhiên, một kiểm tra phòng thủ ở đây cũng không thừa.
        if (m_current >= m_tokens.size())
        {
            // std::cout << "PARSER_UTIL_WARN: peek() called at or beyond end of tokens, returning last token (EOF)." << std::endl;
            return m_tokens.back(); // Should be EOF
        }
        return m_tokens[m_current];
    }

    Token Parser::peek_next() const
    {
        if (m_tokens.empty())
        {
            // std::cerr << "PARSER_UTIL_ERROR: peek_next() called on empty token list." << std::endl;
            throw std::logic_error("Parser::peek_next() called on empty token list.");
        }
        if (m_current + 1 >= m_tokens.size())
        {
            // std::cout << "PARSER_UTIL_WARN: peek_next() at or beyond end-1, returning last token (EOF)." << std::endl;
            return m_tokens.back(); // EOF
        }
        return m_tokens[m_current + 1];
    }

    Token Parser::advance()
    {
        if (!is_at_end())
        {
            m_current++;
        }
        // Luôn trả về token vừa được "tiêu thụ" (consumed)
        return previous();
    }

    bool Parser::is_at_end() const
    {
        if (m_tokens.empty())
            return true;
        // Parser dừng khi token hiện tại là END_OF_FILE.
        return m_tokens[m_current].type == TokenType::END_OF_FILE;
    }

    bool Parser::check(TokenType type) const
    {
        // Sửa: Nếu đã ở cuối file, chỉ trả về true nếu type là END_OF_FILE
        if (is_at_end())
            return type == TokenType::END_OF_FILE;
        return peek().type == type;
    }

    bool Parser::check_next(TokenType type) const
    {
        if (is_at_end() || m_tokens[m_current + 1].type == TokenType::END_OF_FILE)
            return false;
        // peek_next() đã xử lý trường hợp m_current + 1 >= m_tokens.size()
        return peek_next().type == type;
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
        {
            return advance();
        }
        // Nếu không khớp, ném lỗi. error() sẽ set m_had_error = true.
        throw error(peek(), error_message);
    }

    // --- Error Handling and Synchronization ---
    Parser::ParseError Parser::error(const Token &token, const std::string &message)
    {
        std::cerr << "ERROR [Line: " << token.line << ", Col: " << token.column_start << "] ParserError: Syntax error";
        if (token.type == TokenType::END_OF_FILE)
        {
            std::cerr << " at end of file";
        }
        else if (token.type == TokenType::ERROR)
        {
            // If token is ERROR from Lexer, no need to print "at token ..."
        }
        else
        {
            std::cerr << " at token '" << token.lexeme << "'";
        }
        std::cerr << ": " << message << std::endl;

        m_had_error = true;
        return ParseError(token, message);
    }

    void Parser::synchronize()
    {
        // Luôn advance ít nhất một lần nếu chưa ở EOF
        bool at_end = is_at_end();
        if (!at_end)
            advance();
        at_end = is_at_end();

        while (!at_end)
        {
            if (m_current > 0 && previous().type == TokenType::SEMICOLON)
                return;

            switch (peek().type)
            {
            case TokenType::FUNC_KW:
            case TokenType::VAR_KW:
            case TokenType::VAS_KW:
            case TokenType::CONST_KW:
            case TokenType::FOR_KW:
            case TokenType::IF_KW:
            case TokenType::WHILE_KW:
            case TokenType::PRINT_KW:
            case TokenType::RETURN_KW:
            case TokenType::SWITCH_KW:
            case TokenType::TRY_KW:
                return;
            default:
                break;
            }
            // Luôn advance nếu chưa ở EOF
            advance();
            at_end = is_at_end();
        }
    }

    // --- Main Parse Method ---
    AST::StmtList Parser::parse()
    {
        // std::cout << "PARSER_MAIN: Bắt đầu parse()." << std::endl;
        AST::StmtList statements;
        // m_had_error đã được khởi tạo là false trong constructor

        while (!is_at_end())
        {
            // std::cout << "PARSER_MAIN: Vòng lặp chính, peek() = " << debug_token_info(peek()) << std::endl;
            try
            {
                statements.push_back(declaration()); // declaration() là điểm bắt đầu cho mỗi câu lệnh/khai báo cấp cao nhất
                // if (m_had_error) { // Nếu declaration() ném lỗi và được bắt bởi synchronize nội bộ (nếu có)
                //    std::cout << "PARSER_MAIN: Lỗi đã xảy ra trong declaration, đã synchronize, tiếp tục." << std::endl;
                // } else {
                //    std::cout << "PARSER_MAIN: Đã thêm một declaration/statement vào AST. Tổng số hiện tại: " << statements.size() << std::endl;
                // }
            }
            catch (const ParseError &) // ParseError đã được log và m_had_error đã được set bởi error()
            {
                // std::cout << "PARSER_MAIN: Bắt được ParseError từ declaration/statement. Gọi synchronize." << std::endl;
                synchronize(); // Cố gắng phục hồi để parse các câu lệnh tiếp theo
            }
            catch (const std::exception &e_std) // Bắt các lỗi runtime không mong muốn khác từ logic Parser
            {
                std::cerr << "Lỗi hệ thống không mong muốn trong quá trình parse: " << e_std.what() << std::endl;
                m_had_error = true; // Đảm bảo set cờ lỗi
                if (!is_at_end())
                { // Chỉ synchronize nếu chưa ở cuối
                    // std::cout << "PARSER_MAIN: Bắt được std::exception. Gọi synchronize." << std::endl;
                    synchronize();
                }
            }
        }
        // std::cout << "PARSER_MAIN: Kết thúc parse(). Tổng số câu lệnh AST: " << statements.size() << ". Trạng thái lỗi: " << (m_had_error ? "CÓ LỖI" : "KHÔNG CÓ LỖI") << std::endl;
        return statements;
    }

} // namespace Linh