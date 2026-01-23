#include "parser_validation.hpp"

SyntaxError::SyntaxError(const std::string& msg, ParserExitCode code) : std::runtime_error(msg), m_code(code) { }

ParserExitCode const SyntaxError::getCode() const noexcept { return m_code; }

UnknownCommandError::UnknownCommandError(const std::string& msg) : SyntaxError(msg, ParserExitCode::UnknownCommand) { }

UnknownOptionError::UnknownOptionError(const std::string& msg) : SyntaxError(msg, ParserExitCode::UnknownOption) { }

// UnknownFlagError::UnknownFlagError(const std::string& msg) : SyntaxError(msg, ParserExitCode::UnknownFlag) { }

MissingArgumentError::MissingArgumentError(const std::string& msg) : SyntaxError(msg, ParserExitCode::MissingArgument) { }

MissingOptionError::MissingOptionError(const std::string& msg) : SyntaxError(msg, ParserExitCode::MissingArgument) { }

std::ostream& operator<<(std::ostream& stream, ParserExitCode code)
{
    switch (code)
    {
        case ParserExitCode::UnknownCommand: return stream << "UnknownCommand";
        case ParserExitCode::UnknownOption: return stream << "UnknownOption";
        // case ParserExitCode::UnknownFlag: return stream << "UnknownFlag";
        case ParserExitCode::MissingArgument: return stream << "MissingArgument";
        case ParserExitCode::MissingOption: return stream << "MissingOption";
        default: return stream << "Unkown parser exit code";
    }
}
