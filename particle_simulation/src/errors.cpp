#include "errors.hpp"

GLFWError::GLFWError(const std::string& message, ExitCode code) : 
    std::runtime_error(message), m_code(code) { }

ExitCode GLFWError::getCode() const { return m_code; }

GLFWInitFailedError::GLFWInitFailedError(const std::string& m) : GLFWError(m, ExitCode::GLFWInitFailed) { }

GLFWWindowFailedError::GLFWWindowFailedError(const std::string& m) : GLFWError(m, ExitCode::GLFWWindowFailed) { }

std::ostream& operator<<(std::ostream& stream, ExitCode code)
{
    switch(code)
    {
        case ExitCode::GLFWInitFailed: return stream << "GLFWInitFailed";
        case ExitCode::GLFWWindowFailed: return stream << "GLFWWindowFailed";
        default: return stream << "Unknown code";
    }
}