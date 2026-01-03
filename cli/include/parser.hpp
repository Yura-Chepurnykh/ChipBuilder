#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <memory>
#include <exception>

#include "lexer.hpp"

class Parser 
{
public:
    Parser(const std::string&);

private:
    size_t m_currIdx;
    std::vector<std::unique_ptr<Token>> m_tokens;
};

#endif // PARSER_HPP
