#ifndef PARSER_VALIDATION_HPP
#define PARSER_VALIDATION_HPP

#include <iostream>
#include <stdexcept>
#include <string>

enum class ParserExitCode 
{
    UnknownCommand,
    UnknownOption,
    UnknownFlag,
    MissingArgument,
    IncorrectOrder
};

std::ostream& operator<<(std::ostream&, ParserExitCode);

class SyntaxError : public std::runtime_error 
{
public:
    SyntaxError(const std::string&, ParserExitCode);

    ParserExitCode const getCode() const noexcept;

private:
    ParserExitCode m_code;
};

class UnknownCommandError : public SyntaxError 
{
public:
    UnknownCommandError(const std::string&);
};

class UnknownOptionError : public SyntaxError 
{
public:
    UnknownOptionError(const std::string&);
};

class UnknownFlagError : public SyntaxError 
{
public:
    UnknownFlagError(const std::string&);
};

class MissingArgumentError : public SyntaxError 
{
public:
    MissingArgumentError(const std::string&);
};

class IncorrectOrderError : public SyntaxError 
{
public:
    IncorrectOrderError(const std::string&);
};

#endif // PARSER_VALIDATION_HPP