#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <vector>
#include <memory>
#include "context.hpp"

// singleton pattern
class Application 
{
public:
    static Application& instance();
    void add(std::unique_ptr<IContext>);

private:
    Application() = default;

    std::vector<std::unique_ptr<IContext>> m_contexts;
};

#include "application.tpp"

#endif // APPLICATION_HPP