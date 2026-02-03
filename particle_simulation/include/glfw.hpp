#ifndef GLFW_HPP
#define GLFW_HPP

#include <GLFW/glfw3.h>

// wrap glfw initialization in a Singleton RAII class
class GLFW
{
public:
    static GLFW& instance();

    GLFW(GLFW&) = delete;
    GLFW& operator=(GLFW&) = delete;

    GLFW(GLFW&&) = delete;
    GLFW& operator=(GLFW&&) = delete;

    ~GLFW();

private:
    GLFW();
};

#endif // GLFW_HPP
