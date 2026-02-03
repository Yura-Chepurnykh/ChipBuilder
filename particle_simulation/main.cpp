#include <iostream>
#include "glfw.hpp"
#include "exceptions.hpp"

int main()
{
    try {
        GLFW::instance();
    }
    catch(const GLFWInitFailed& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
        return static_cast<unsigned int>(e.getCode());
    }
    catch(const std::runtime_error& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
