#include "router.hpp"

Router::Router()
{
    registerCommand<CreateNMOSCommand>("create-nmos");
    registerCommand<CreatePMOSCommand>("create-pmos");
    registerCommand<CreateSubstrateCommand>("create-substrate");
    registerCommand<CreateSourceCommand>("create-source");
    registerCommand<CreateDrainCommand>("create-drain");
    registerCommand<CreateGateCommand>("create-gate");
    registerCommand<CreatePolyCommand>("create-poly");
    registerCommand<CreateOxideCommand>("create-oxide");
}

template<typename CommandType>
void Router::registerCommand(const std::string& name)
{
    static_assert(std::is_base_of<Command, CommandType>::value, "CommandType must derive from Command");

    factoriesMap[name] = [](const ObjectParameter& param)
    {
        return std::make_unique<CommandType>(param);
    };
}
