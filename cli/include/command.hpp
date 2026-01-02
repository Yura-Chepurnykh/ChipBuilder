#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <vector>
#include <tuple>
#include "token.hpp"

namespace Action 
{
    struct Create { };
    struct Remove { };
    struct GoTo { };
    struct Read { };
}

namespace Entity 
{
    struct Transistor { };
    struct BJT { };
    struct CMOS { };
    struct NMOS { };
    struct PMOS { };
    struct Substrate { };
    struct Via { };
    struct EpiLayer { };
    struct Diffusion { };
    struct Gate { };
    struct OxideLayer { };
    struct Implant { };
    struct Metal { };
}

template<typename... Commands>
struct CommandScheme
{
    struct OptionToArgument 
    {
        Token option;
        Token argument;
    };

    using Path = std::tuple<Commands...>;
    
    std::vector<OptionToArgument> optionsToArguments;
    std::vector<Token> flags;
};

#endif // COMMAND_HPP
