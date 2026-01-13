#include "lexer_validation.hpp"

LexerError::LexerError(const std::string& msg, LexerExitCode code) : m_code(code), std::runtime_error(msg) { }    

LexerExitCode const LexerError::getCode() const noexcept { return m_code; }

EmptyCommandError::EmptyCommandError(const std::string& msg) : LexerError(msg, LexerExitCode::EmptyCommand) { }

UnexpectedSymbolError::UnexpectedSymbolError(const std::string& msg) : LexerError(msg, LexerExitCode::UnexpectedSymbol) { }

UnterminatedStringError::UnterminatedStringError(const std::string& msg) : LexerError(msg, LexerExitCode::UnterminatedString) { }

EmptyDashWordError::EmptyDashWordError(const std::string& msg) : LexerError(msg, LexerExitCode::EmptyDashWord) { }

EmptyDoubleDashWordError::EmptyDoubleDashWordError(const std::string& msg) : LexerError(msg, LexerExitCode::EmptyDoubleDashWord) { }

EmptyStringError::EmptyStringError(const std::string& msg) : LexerError(msg, LexerExitCode::EmptyString) { }

IllegalWordError::IllegalWordError(const std::string& msg) : LexerError(msg, LexerExitCode::IllegalWord) { }

std::ostream& operator<<(std::ostream& stream, LexerExitCode code)
{
    switch(code)
    {
        case LexerExitCode::Success: return stream << "Success";
        case LexerExitCode::UnterminatedString: return stream << "UnterminatedString Code";
        case LexerExitCode::EmptyCommand: return stream << "EmptyCommand Code";
        case LexerExitCode::EmptyDashWord: return stream << "EmptyDashWord Code";
        case LexerExitCode::EmptyDoubleDashWord: return stream << "EmptyDoubleDashWord Code";
        case LexerExitCode::IllegalWord: return stream << "IllegalWord Code";
        case LexerExitCode::UnexpectedSymbol: return stream << "UnexpectedSymbol Code";
        default: return stream << "Unknown Code";
    }
}