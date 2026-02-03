#include <iostream>
#include "window.hpp"
#include "glfw.hpp"
#include "errors.hpp"

int main()
{
    try {
        GLFW::instance();
        Window window = WindowBuilder().setWidth(1600).setHeight(900).setTitle("Particle Simulation").setMonitor(nullptr).setShare(nullptr).build();
        std::cout << window << std::endl;
    }
    catch(const GLFWInitFailedError& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
        return static_cast<unsigned int>(e.getCode());
    }
    catch(const GladInitFailedError& e)
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
