#ifndef LEXER_HPP
#define LEXER_HPP

#include <iostream>
#include <string>
#include <cstddef>
#include "token.hpp"

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
