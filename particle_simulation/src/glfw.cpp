#include <iostream>
#include "glfw.hpp"
#include "errors.hpp"

GLFW::GLFW()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialized glfw" << std::endl;
        throw GLFWInitFailedError("GLFW initialization failed");
    }
}

GLFW::~GLFW()
{
    glfwTerminate();
}

GLFW& GLFW::instance() 
{
    static GLFW instance;
    return instance;
}
