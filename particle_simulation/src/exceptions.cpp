#include "exceptions.hpp"

GLFWInitFailed::GLFWInitFailed(const std::string& message) : 
    std::runtime_error(message), m_code(ExitCode::GLFWInitFailed) { }

ExitCode const GLFWInitFailed::getCode() const { return m_code; }

std::ostream& operator<<(std::ostream& stream, ExitCode code)
{
    switch(code)
    {
        case ExitCode::GLFWInitFailed: return stream << "GLFWInitFailed";
        default: return stream << "Unknown code";
    }
}