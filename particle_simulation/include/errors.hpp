#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <iostream>
#include <stdexcept>
#include <string>

enum class ExitCode
{
    GLFWInitFailed = 1,
    GLFWWindowFailed = 2
};

std::ostream& operator<<(std::ostream&, ExitCode);

class GLFWError : public std::runtime_error 
{
public:
    ExitCode getCode() const;

protected:
    explicit GLFWError(const std::string&, ExitCode);

private:
    ExitCode m_code;
};

class GLFWInitFailedError : public GLFWError
{
public:
    explicit GLFWInitFailedError(const std::string&);
};

class GLFWWindowFailedError : public GLFWError
{
public:
    explicit GLFWWindowFailedError(const std::string&);
};

#endif // EXCEPTIONS_HPP
