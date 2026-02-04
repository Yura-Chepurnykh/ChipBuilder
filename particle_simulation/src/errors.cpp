#include "errors.hpp"

GLError::GLError(const std::string& message, ExitCode code) : 
    std::runtime_error(message), m_code(code) { }

ExitCode GLError::getCode() const { return m_code; }

GLFWInitFailedError::GLFWInitFailedError(const std::string& m) : GLError(m, ExitCode::GLFWInitFailed) { }

GLFWWindowFailedError::GLFWWindowFailedError(const std::string& m) : GLError(m, ExitCode::GLFWWindowFailed) { }

GladInitFailedError::GladInitFailedError(const std::string& m) : GLError(m, ExitCode::GladInitFailed) { }

ShaderLinkFailedError::ShaderLinkFailedError(const std::string& m) : GLError(m, ExitCode::ShaderLinkFailed) { }

std::ostream& operator<<(std::ostream& stream, ExitCode code)
{
    switch(code)
    {
        case ExitCode::GLFWInitFailed: return stream << "GLFWInitFailed";
        case ExitCode::GLFWWindowFailed: return stream << "GLFWWindowFailed";
        case ExitCode::GladInitFailed: return stream << "GladInitFailed";
        case ExitCode::ShaderLinkFailed: return stream << "ShaderLinkFailed";
        default: return stream << "Unknown code";
    }
}