#include "controller.hpp"

Controller::Controller(std::unique_ptr<View> view, std::unique_ptr<IContext> context, std::unique_ptr<IRouter> router) :
    m_view(std::move(view)), m_context(std::move(context)), m_router(std::move(router)) { }

void Controller::run() 
{
    while (true)
    {
        View view;
        std::string input = view();
        Parser parser(input);
        auto scheme = parser.parse();
        
        auto factory = m_router->dispatchCommand(scheme);
        std::shared_ptr<IParameterObject> args = m_router->extractArgs(scheme);

        std::shared_ptr<ICommand> cmd = factory(args);
        cmd->execute();
    }
}