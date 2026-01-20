#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <memory>
#include <exception>
#include <algorithm>
#include <array>
#include <iterator>

#include "lexer.hpp"
#include "command_scheme.hpp"
#include "parser_validation.hpp"

class Parser 
{
public:
    Parser(const std::string&);
    CommandScheme parse();

private:
    size_t m_currIdx;
    std::vector<Token> m_tokens;
};

#endif // PARSER_HPP
