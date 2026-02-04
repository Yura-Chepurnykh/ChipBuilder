#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <iostream>
#include <stdexcept>
#include <string>

enum class ExitCode
{
    GLFWInitFailed   = 1,
    GLFWWindowFailed = 2,
    GladInitFailed   = 3,
    ShaderLinkFailed = 4,
};

std::ostream& operator<<(std::ostream&, ExitCode);

class GLError : public std::runtime_error 
{
public:
    ExitCode getCode() const;

protected:
    explicit GLError(const std::string&, ExitCode);

private:
    ExitCode m_code;
};

class GLFWInitFailedError : public GLError
{
public:
    explicit GLFWInitFailedError(const std::string&);
};

class GLFWWindowFailedError : public GLError
{
public:
    explicit GLFWWindowFailedError(const std::string&);
};

class GladInitFailedError : public GLError 
{
public: 
    explicit GladInitFailedError(const std::string&);
};

class ShaderLinkFailedError : public GLError 
{
public:
    explicit ShaderLinkFailedError(const std::string&);
};

#endif // EXCEPTIONS_HPP
