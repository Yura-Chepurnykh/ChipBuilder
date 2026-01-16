#include "view.hpp"

std::string View::operator()()
{
    std::cout << "> ";
    std::string input;
    std::getline(std::cin, input);

    return input;
}