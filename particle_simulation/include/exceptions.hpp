#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <iostream>
#include <stdexcept>
#include <string>

enum class ExitCode
{
    GLFWInitFailed = 1
};

std::ostream& operator<<(std::ostream&, ExitCode);

class GLFWInitFailed : public std::runtime_error
{
public:
    explicit GLFWInitFailed(const std::string&);
    ExitCode const getCode() const;

private:
    ExitCode m_code;
};

#endif // EXCEPTIONS_HPP
