#include "router.hpp"

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
    template<typename Semi>
    std::unique_ptr<IParameterObject<NT>> Router<NT>::createSubstrate(const CommandScheme& scheme, std::string_view& defaultDopant)
    {
        if (auto it = std::ranges::find_if (
            scheme.optionsToArguments, [](const CommandScheme::OptionToArgument& optToArg) {
                 return optToArg.option.content == "--dopant";
        }); it != scheme.optionsToArguments.end())
        {
            auto itDopantMap = DopantsFactory<Semi, NT>::map.find(it->second);

            if (itDopantMap == DopantsFactory<Semi, NT>::map.end())
                throw std::runtime_error("Unexpected dopant");

            return itDopantMap->second();
        }
        
        return DopantsFactory<Semi, NT>::map.at(defaultDopant);
    };

    template<NumericType NT>
        void Router<NT>::fillArgs (IParameterObject<NT>* param, const CommandScheme& scheme)
    {
        for (const auto& [option, argument] : scheme.optionsToArguments)
        {
            if (option.content == "--pos") param->rect.position = argument;
            else if (option.content == "--width") param->width = argument;
            else if (option.content == "--height") param->height = argument;
            else if (option.content == "--layer") param->layer = argument;
        }
    };

template<NumericType NT>
void Router<NT>::registerParams(const CommandScheme& scheme)
{  
    m_paramsFactories["create-nmos"] = []()
    {
        return std::make_unique<CreateMosParams<NType, NT>>;
    };

    m_paramsFactories["create-pmos"] = []()
    {
        return std::make_unique<CreateMosParams<PType, NT>>;
    };

    m_paramsFactories["create-n-substrate"] = [&scheme]()
    {   
        auto param = createSubstrate<NType>("phosphorus");
        fillArgs(param.get(), scheme.optionsToArguments);
        return param;
    };

    m_paramsFactories["create-p-substrate"] = [&scheme]()
    { 
        auto param = createSubstrate<PType>("boron");
        fillArgs(param.get(), scheme.optionsToArguments);
        return param;
    };
}

template<NumericType NT>
std::optional<typename Router<NT>::CommandFactory> Router<NT>::dispatch(const std::string& command)
{
    if (auto iter = m_commandsFactories.find(command); iter == m_commandsFactories.end())
        return std::nullopt;
    else
        return iter->second;
}