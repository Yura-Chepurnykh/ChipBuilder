#ifndef LEXER_VALIDATION_HPP
#define LEXER_VALIDATION_HPP

#include <stdexcept>
#include <iostream>

enum class LexerExitCode 
{
    Success,
    EmptyCommand,
    UnexpectedSymbol,
    UnterminatedString,
    EmptyDashWord,
    EmptyDoubleDashWord,
    EmptyString,
    IllegalWord
};

std::ostream& operator<<(std::ostream& stream, LexerExitCode);

class LexerError : public std::runtime_error
{
public:
    explicit LexerError(const std::string&, LexerExitCode);
    LexerExitCode const getCode() const noexcept;

private:
    LexerExitCode m_code;
};

class EmptyCommandError : public LexerError 
{
public:
    explicit EmptyCommandError(const std::string&);
};

class UnexpectedSymbolError : public LexerError
{
public:
    explicit UnexpectedSymbolError(const std::string&);
};

class UnterminatedStringError : public LexerError 
{
public:
    explicit UnterminatedStringError(const std::string&);
};

class EmptyDashWordError : public LexerError
{
public:
    explicit EmptyDashWordError(const std::string&);
};

class EmptyDoubleDashWordError : public LexerError 
{
public:
    explicit EmptyDoubleDashWordError(const std::string&);
};

class EmptyStringError : public LexerError
{
public:
    explicit EmptyStringError(const std::string&);
};

class IllegalWordError : public LexerError
{
public:
    explicit IllegalWordError(const std::string&);
};

#endif // LEXER_VALIDATION_HPP