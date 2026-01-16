#include <iostream>
#include "parser.hpp"

int main()
{
    std::string cmd = R"(command subcommand --option1 "arg1" --option2 "arg2" -v)";
    Parser parser(cmd);
    auto scheme = parser.parse();


    std::cout << scheme.command << std::endl;
    // std::cout << scheme.subCommand << std::endl;

    for (const auto& [option, argument] : scheme.optionsToArguments)
    {
        std::cout << option << " " << argument << std::endl;
    }

    std::cout << scheme.flag << std::endl;
}

