#include "router.hpp"
#include "traits.hpp"

template<NumericType NT>
Router<NT>::Router()
{
    registerCommand<CreateMOSCommand<NType>>("create-nmos");
    
    registerCommand<CreateMOSCommand<PType>>("create-pmos");

    registerCommand<CreateSubstrateCommand<NType, Phosphorus>>("create-n-substrate");
    registerCommand<CreateSubstrateCommand<PType, Boron>>("create-p-substrate");

    registerCommand<CreateDiffusionCommand<NType, Source<NType, Phosphorus>, Phosphorus>>("create-n-source");
    registerCommand<CreateDiffusionCommand<PType, Source<PType, Boron>, Boron>>("create-p-source");
    
    registerCommand<CreateDiffusionCommand<NType, Drain<NType, Phosphorus>, Phosphorus>>("create-n-drain");
    registerCommand<CreateDiffusionCommand<PType, Drain<PType, Boron>, Boron>>("create-p-drain");
    
    registerCommand<CreatePolyCommand<NMOS>>("create-nmos-poly");
    registerCommand<CreatePolyCommand<PMOS>>("create-pmos-poly");
    
    registerCommand<CreateOxideCommand<NMOS>>("create-nmos-oxide");
    registerCommand<CreateOxideCommand<PMOS>>("create-pmos-oxide");
}

template<NumericType NT>
template<typename CommandType>
void Router<NT>::registerCommand(const std::string& command)
{
    static_assert(std::is_base_of<ICommand, CommandType>::value, "CommandType must inherit from Command");

    m_commandsFactories[command] = [](std::shared_ptr<IParameterObject> args)
    {
        const auto& p = dynamic_pointer_cast<ParameterObject<NT>>(args);
        return std::make_unique<CommandType>(std::move(p));
    };
}

template<NumericType NT>
std::shared_ptr<IParameterObject> Router<NT>::extractArgs(CommandScheme& scheme)
{  
    if (scheme.command.content == "create-nmos") 
        return std::make_shared<CreateMosParams<NType, NT>>();

    else if (scheme.command.content == "create-pmos") 
        return std::make_shared<CreateMosParams<PType, NT>>();

    else if (scheme.command.content == "create-n-substrate") 
    {   
        auto param = createLayer<NType, Substrate>(scheme, "phosphorus");
        fillArgs(param.get(), scheme);
        return param;
    }

    else if (scheme.command.content == "create-p-substrate") 
    { 
        auto param = createLayer<PType, Substrate>(scheme, "boron");
        fillArgs(param.get(), scheme);
        return param;
    }

    else if (scheme.command.content == "create-n-source") 
    {
        auto param = createLayer<NType, Source>(scheme, "phosphorus");
        fillArgs(param.get(), scheme);
        return param;
    }

    else if (scheme.command.content == "create-p-source") 
    {
        auto param = createLayer<PType, Source>(scheme, "phosphorus");
        fillArgs(param.get(), scheme);
        return param;
    }

    else if (scheme.command.content == "create-n-drain") 
    {
        auto param = createLayer<PType, Drain>(scheme, "phosphorus");
        fillArgs(param.get(), scheme);
        return param;
    }

    else if (scheme.command.content == "create-p-drain") 
    {
        auto param = createLayer<PType, Drain>(scheme, "boron");
        fillArgs(param.get(), scheme);
        return param;
    }

    // else if (scheme.command.content == "create-oxide") 
    // {
    //     auto param = createLayer<Oxide>(scheme, "");
    //     fillArgs(param.get(), scheme.optionsToArguments);
    //     return param;
    // }

    // else if (scheme.command.content == "create-poly") 
    // {
    //     auto param = createLayer<PolySilicon>(scheme, "");
    //     fillArgs(param.get(), scheme.optionsToArguments);
    //     return param;
    // }
}

template<NumericType NT>
template<typename Semi, template<typename ...> class L>
std::shared_ptr<ParameterObject<NT>> Router<NT>::createLayer(CommandScheme& scheme, const std::string& defaultDopant)
{
    static_assert(is_semi<Semi>::value, "Semi must be NType or PType");

    if (auto it = std::ranges::find_if (
        scheme.optionsToArguments, [](const CommandScheme::OptionToArgument& optToArg) {
            return optToArg.option.content == "--dopant";
    }); it != scheme.optionsToArguments.end())
    {
        auto itDopantMap = DopantsFactory<Semi, NT, L>::map.find(it->argument.content);

        if (itDopantMap == DopantsFactory<Semi, NT, L>::map.end())
            throw std::runtime_error("Unexpected dopant");

        return itDopantMap->second;
    }
        
    return DopantsFactory<Semi, NT, L>::map.at(defaultDopant);
};

template<NumericType NT>
void Router<NT>::fillArgs(ParameterObject<NT>* param, CommandScheme& scheme)
{
    for (const auto& [option, argument] : scheme.optionsToArguments)
    {
        // if (option.content == "--pos") param->rect.position = argument;
        if (option.content == "--width") param->rect.width = std::stoi(argument.content);
        else if (option.content == "--height") param->rect.height = std::stoi(argument.content);
        // else if (option.content == "--layer") param->layer = argument;
    }
};

template<NumericType NT>
Router<NT>::CommandFactory Router<NT>::dispatchCommand(CommandScheme& scheme)
{
    // if (auto iter = m_commandsFactories.find(scheme.command.content); iter == m_commandsFactories.end())
    //     return std::nullopt;
    // else 
    //     return iter->second;

    return m_commandsFactories.find(scheme.command.content)->second;
}