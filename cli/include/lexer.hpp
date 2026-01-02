#ifndef LEXER_HPP
#define LEXER_HPP

#include <iostream>
#include <string>
#include <cstddef>

enum class Type 
{
    Word,
    DashWord,
    DoubleDashWord,
    String,
    EofToken
};

std::ostream& operator<<(std::ostream&, Type);

struct Token 
{
    size_t pos;
    std::string content;
    Type type;
};

std::ostream& operator<<(std::ostream&, const Token& token);

class Lexer 
{
public:
    Lexer(const std::string&) noexcept;
    Token getCurrentToken();

private:
    size_t m_pos;
    std::string m_text;
};

#endif // LEXER_HPP
