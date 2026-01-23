#include <algorithm>
#include "lexer_validation.hpp"
#include "parser.hpp"

Lexer::Lexer(const std::string& text) noexcept : m_text(text), m_pos(0) { }

Token Lexer::getCurrentToken() 
{
    auto validateWord = [](const std::string& tok) 
    {
        if (std::isdigit(tok.front()))
            throw IllegalWordError(std::string("Invalid identifier '" + tok + "': Identifiers must not start with a digit"));

        for (char c : tok)
        {
            if (!std::isalnum(static_cast<unsigned char>(c)) && static_cast<unsigned char>(c) != '-')
                throw UnexpectedSymbolError(std::string("Unexpected symbol '") + c + "' in identifier '" + tok + "'");
        }
    };

    if (m_text.empty())
        throw EmptyCommandError(std::string("The command is empty"));

    static constexpr char delimiter = ' ';
    
    m_pos = m_text.find_first_not_of(delimiter, m_pos);

    if (m_pos == std::string::npos)
        return Token { m_pos, "", Type::EofToken };

    size_t end = m_text.find_first_of(delimiter, m_pos);

    size_t length = (end == std::string::npos ? m_text.length() : end) - m_pos;

    std::string token = m_text.substr(m_pos, length);

    if (token.front() == '"')
    {
        size_t iter = m_text.find("\"", m_pos + 1);

        if (iter != std::string::npos)
        {
            std::string content = m_text.substr(m_pos, iter - m_pos + 1);
            
            if (content.length() <= 2)
                throw EmptyStringError("Empty string literal \"\" is not allowed");

            m_pos = iter + 1;
            return Token { m_pos, content.substr(1, content.length() - 2), Type::String };
        }

        throw UnterminatedStringError(std::string("Unterminated string literal: ") + token +  "missing closing \"");
    }

    size_t begin = m_pos;
    m_pos = (end == std::string::npos ? m_text.size() : end);
    
    if (token.length() >= 2 && token.substr(0, 2) == "--")
    {
        if (token.length() == 2)
            throw EmptyDoubleDashWordError(std::string("Double dash word is invalid: Expected name after '--'"));

        validateWord(token.substr(2));

        return Token { begin, token, Type::DoubleDashWord };
    }
    
    else if (token.length() >= 1 && token.front() == '-')
    {
        if (token.length() == 1)
            throw EmptyDashWordError(std::string("Dash word is invalid: Expected name after '-'"));

        validateWord(token.substr(1));

        return Token { begin, token, Type::DashWord };
    }
    
    validateWord(token);

    return Token { begin, token, Type::Word };
}
