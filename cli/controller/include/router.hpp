#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>
#include <string_view>
#include <memory>
#include <functional>
#include <unordered_map>
#include <optional>
#include <algorithm>
#include <map>
#include <type_traits>

#include "concepts.hpp"
#include "parameter_object.hpp"
#include "dopant_material.hpp"
#include "commands.hpp"
#include "command_scheme.hpp"
#include "layers.hpp"
#include "circuits.hpp"

template<typename SemiConductorType, NumericType NT>
struct DopantsFactory;

template<NumericType NT>
struct DopantsFactory<NType, NT>
{
    inline static const std::map<std::string, std::function<std::unique_ptr<IParameterObject<NT>>()>> map
    {
        { "phosphorus", []() { return std::make_unique<CreateParams<Substrate<NType, Phosphorus>, NType, Phosphorus, NT>>(); }},
        { "arsenic",    []() { return std::make_unique<CreateParams<Substrate<NType, Arsenic>, NType, Arsenic, NT>>(); }},
        { "antimony",   []() { return std::make_unique<CreateParams<Substrate<NType, Antimony>, NType, Antimony, NT>>(); }},        
    };
};

template<NumericType NT>
struct DopantsFactory<PType, NT>
{
    inline static const std::map<std::string, std::function<std::unique_ptr<IParameterObject<NT>>()>> map
    {     
        { "boron", []() { return std::make_unique<CreateParams<Substrate<PType, Boron>, PType, Boron, NT>>(); }},        
    };
};

struct IRouter 
{
    virtual ~IRouter() = default;
    virtual std::optional<std::unique_ptr<ICommand>> dispatch(const std::string&) = 0;
};

template<NumericType NT>
class Router : public IRouter
{
public:
    using CommandFactory = std::function<std::unique_ptr<ICommand>(const IParameterObject<NT>&)>;

    using ParamFactory = std::function<std::unique_ptr<IParameterObject<NT>>()>;

    Router();
    
    std::optional<CommandFactory> dispatch(const std::string&);

private:
    template<typename CommandType>
    void registerCommand(const std::string& command)
    {
        static_assert(std::is_base_of<ICommand, CommandType>::value, "CommandType must inherit from Command");

        m_commandsFactories[command] = [](const IParameterObject<NT>& param)
        {
            return std::make_unique<CommandType>(param);
        };
    }

    template<typename Semi>
    std::unique_ptr<IParameterObject<NT>> createSubstrate(const CommandScheme&, std::string_view&);

    void fillArgs(IParameterObject<NT>*, const CommandScheme&);
    
    void registerParams(const CommandScheme&);

    std::unordered_map<std::string, ParamFactory> m_paramsFactories;
    std::unordered_map<std::string, CommandFactory> m_commandsFactories;
};

#include "router.tpp"

#endif // ROUTER_HPP