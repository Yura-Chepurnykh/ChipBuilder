#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <iostream>
#include <memory>
#include <string>
#include <GLFW/glfw3.h>

class Window 
{
public:
    Window(int, int, const std::string&, GLFWmonitor*, GLFWwindow*);

    ~Window();

    void setWidth(int) noexcept;
    int getWidth() const noexcept;

    void setHeight(int) noexcept;
    int getHeight() const noexcept;

    void setTitle(const std::string&) noexcept;
    std::string getTitle() const noexcept;

    void setMonitor(GLFWmonitor*);
    GLFWmonitor* getMonitor() const;

    void setShare(GLFWwindow*);
    GLFWwindow* getShare() const;

private:
    int m_width, m_height;
    std::string m_title;
    GLFWmonitor* m_monitor;
    GLFWwindow *m_window, *m_shareResources;
};

std::ostream& operator<<(std::ostream& stream, const Window&);

class WindowBuilder
{
public:
    struct Context
    {
        int width, height;
        std::string title;
        GLFWmonitor* monitor;
        GLFWwindow* shareResources;
    };

    WindowBuilder& setWidth(int);
    WindowBuilder& setHeight(int);
    WindowBuilder& setTitle(const std::string&);
    WindowBuilder& setMonitor(GLFWmonitor*);
    WindowBuilder& setShare(GLFWwindow*);
    Window build();

private:
    Context m_context;
};

#endif // WINDOW_HPP