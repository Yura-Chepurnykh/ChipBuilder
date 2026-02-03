#include "window.hpp"
#include "errors.hpp"

Window::Window(
    int w = 400,
    int h = 600, 
    const std::string& t = "untitled",
    GLFWmonitor* m = nullptr, 
    GLFWwindow* s = nullptr
) : 
    m_width(w),
    m_height(h),
    m_title(t), 
    m_monitor(m), 
    m_shareResources(s)
{
    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), m_monitor, m_shareResources);

    if (!m_window)
    {
        std::cerr << "Window initialization failed" << std::endl;
        throw GLFWWindowFailedError("Window initialization failed");
    }

    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Glad initialization failed" << std::endl;
        throw GladInitFailedError("Glad initializaion failed");
    }

    while (!glfwWindowShouldClose(m_window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
}

void Window::setWidth(int w) noexcept { m_width = w; } 
int Window::getWidth() const noexcept { return m_width; }

void Window::setHeight(int h) noexcept { m_height = h; }
int Window::getHeight() const noexcept { return m_height; }

void Window::setTitle(const std::string& title) noexcept { m_title = title; } 
std::string Window::getTitle() const noexcept { return m_title; }

void Window::setMonitor(GLFWmonitor* m) { m_monitor = m; }
GLFWmonitor* Window::getMonitor() const { return m_monitor; }

void Window::setShare(GLFWwindow* s) { m_shareResources = s; }
GLFWwindow* Window::getShare() const { return m_shareResources; }

WindowBuilder& WindowBuilder::setWidth(int w) 
{
    m_context.width = w;
    return *this;
}

WindowBuilder& WindowBuilder::setHeight(int h)
{
    m_context.height = h;
    return *this;
}

WindowBuilder& WindowBuilder::setTitle(const std::string& title)
{
    m_context.title = title;
    return *this;
}

WindowBuilder& WindowBuilder::setMonitor(GLFWmonitor* monitor)
{
    m_context.monitor = monitor;
    return *this;
}

WindowBuilder& WindowBuilder::setShare(GLFWwindow* share)
{
    m_context.shareResources = share;
    return *this;
}

Window WindowBuilder::build()
{
    return Window (
        m_context.width, 
        m_context.height,
        m_context.title, 
        m_context.monitor, 
        m_context.shareResources
    );
}

std::ostream& operator<<(std::ostream& stream, const Window& w)
{
    stream << "(" << w.getWidth() << ", " << w.getHeight() << ", " << w.getTitle() << ")";  
    return stream;
}