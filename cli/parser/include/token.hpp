#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <iostream>

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

#endif // TOKEN_HPP
