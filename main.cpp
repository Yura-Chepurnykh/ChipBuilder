#include <iostream>
// #include "parser.hpp"
#include "controller.hpp"
#include "view.hpp"
#include "context.hpp"
#include "router.hpp"
#include "circuits.hpp"

int main()
{
    // std::string cmd = R"(create-p-substrate --width "100nm" --height "120nm" --pos "300, 350")";
    // Parser parser(cmd);
    // auto scheme = parser.parse();


    // std::cout << scheme.command << std::endl;
    // // std::cout << scheme.subCommand << std::endl;

    // for (const auto& [option, argument] : scheme.optionsToArguments)
    // {
    //     std::cout << option << " " << argument << std::endl;
    // }

    auto context = std::make_unique<MOSContext<NMOS>>();;
    auto view = std::make_unique<View>();
    auto router = std::make_unique<Router<int>>();
    Controller controller(std::move(view), std::move(context), std::move(router));
    controller.run();
}

