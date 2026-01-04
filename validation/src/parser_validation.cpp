#include "parser_validation.hpp"

SyntaxError::SyntaxError(const std::string& msg, ParserExitCode code) : std::runtime_error(msg), m_code(code) { }

ParserExitCode const SyntaxError::getCode() const noexcept { return m_code; }

UnknownCommandError::UnknownCommandError(const std::string& msg) : SyntaxError(msg, ParserExitCode::UnknownCommand) { }

UnknownOptionError::UnknownOptionError(const std::string& msg) : SyntaxError(msg, ParserExitCode::UnknownOption) { }

UnknownFlagError::UnknownFlagError(const std::string& msg) : SyntaxError(msg, ParserExitCode::UnknownFlag) { }

MissingArgumentError::MissingArgumentError(const std::string& msg) : SyntaxError(msg, ParserExitCode::MissingArgument) { }

IncorrectOrderError::IncorrectOrderError(const std::string& msg) : SyntaxError(msg, ParserExitCode::IncorrectOrder) { }

std::ostream& operator<<(std::ostream& stream, ParserExitCode code)
{
    switch (code)
    {
        case ParserExitCode::UnknownCommand: return stream << "UnknownCommand";
        case ParserExitCode::UnknownOption: return stream << "UnknownOption";
        case ParserExitCode::UnknownFlag: return stream << "UnknownFlag";
        case ParserExitCode::IncorrectOrder: return stream << "IncorrectOrder";
        case ParserExitCode::MissingArgument: return stream << "MissingArgument";
        default: return stream << "Unkown parser exit code";
    }
}