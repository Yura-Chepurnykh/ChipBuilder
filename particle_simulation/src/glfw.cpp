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
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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
