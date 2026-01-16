#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <memory>

#include "view.hpp"
#include "context.hpp"
#include "router.hpp"
#include "parser.hpp"

class Controller 
{
public:
    Controller(std::unique_ptr<View>, std::unique_ptr<IContext>, std::unique_ptr<IRouter>);
    void run();

private:
    std::unique_ptr<IRouter> m_router;
    std::unique_ptr<View> m_view;
    std::unique_ptr<IContext> m_context; 
};

// #include "controller.tpp"

#endif // CONTROLLER_HPP