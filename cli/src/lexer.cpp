#include <algorithm>
#include "parser.hpp"

Lexer::Lexer(const std::string& text) noexcept : m_text(text), m_pos(0) { }

Token Lexer::getCurrentToken() 
{
    if (m_text.empty())
        throw std::runtime_error("The command is empty");

    static std::string delimeter = " ";
    
    m_pos = m_text.find_first_not_of(delimeter, m_pos);

    if (m_pos == std::string::npos)
        return Token { m_pos, "", Type::EofToken };

    size_t end = m_text.find_first_of(delimeter, m_pos);

    size_t length = (end == std::string::npos ? m_text.length() : end) - m_pos;

    std::string token = m_text.substr(m_pos, length);

    if (token.front() == '"')
    {
        size_t iter = m_text.find("\"", m_pos + 1);

        if (iter != std::string::npos)
        {
            std::string content = m_text.substr(m_pos, iter - m_pos + 1);
            m_pos = iter + 1;
            return Token { m_pos, content, Type::String };
        }

        throw std::runtime_error("The quotation mark is not closed");
    }

    size_t begin = m_pos;
    m_pos = (end == std::string::npos ? m_text.size() : end);
    
    if (token.substr(0, 2) == "--")
    {
        return Token { begin, token, Type::DoubleDashWord };
    }
    
    else if (token.front() == '-')
    {
        return Token { begin, token, Type::DashWord };
    }
    
    return Token { begin, token, Type::Word };
}

std::ostream& operator<<(std::ostream& stream, Type type)
{
    switch (type)
    {
        case Type::Word: return stream << "Word";
        case Type::DashWord: return stream << "Dash-Word";
        case Type::DoubleDashWord: return stream << "Double-Dash-Word";
        case Type::String: return stream << "String";
        default: return stream << "Unknown token";
    }
}

std::ostream& operator<<(std::ostream& stream, const Token& token)
{
    return stream << "(" << token.pos << ", " << token.content << ", " << token.type << ")"; 
}
