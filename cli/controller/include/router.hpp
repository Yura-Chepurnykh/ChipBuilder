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

template<typename S, NumericType NT, template<typename ...> class L>
struct DopantsFactory;

template<NumericType NT, template<typename ...> class L>
struct DopantsFactory<NType, NT, L>
{
    template<typename... Args>
    using Layer = L<Args...>;

    inline static const std::map<std::string, std::shared_ptr<ParameterObject<NT>>> map
    {
        { "phosphorus", std::make_shared<CreateParams<Layer<NType, Phosphorus>, NType, Phosphorus, NT>>() },
        { "arsenic",    std::make_shared<CreateParams<Layer<NType, Arsenic>,    NType, Arsenic,    NT>>() },
        { "antimony",   std::make_shared<CreateParams<Layer<NType, Antimony>,   NType, Antimony,   NT>>() },       
    };
};

template<NumericType NT, template<typename ...> class L>
struct DopantsFactory<PType, NT, L>
{
    template<typename... Args>
    using Layer = L<Args...>;

    inline static const std::map<std::string, std::shared_ptr<ParameterObject<NT>>> map
    {     
        { "boron", std::make_shared<CreateParams<Layer<PType, Boron>, PType, Boron, NT>>() }, 
    };
};

struct IRouter 
{
    virtual ~IRouter() = default;
    using CommandFactory = std::function<std::unique_ptr<ICommand>(std::shared_ptr<IParameterObject>)>;

    virtual CommandFactory dispatchCommand(CommandScheme&) = 0;
    virtual std::shared_ptr<IParameterObject> extractArgs(CommandScheme&) = 0;
};

template<NumericType NT>
class Router : public IRouter
{
public:
    Router();
    
    virtual CommandFactory dispatchCommand(CommandScheme&) override;
    std::shared_ptr<IParameterObject> extractArgs(CommandScheme&) override;

private:
    template<typename CommandType>
    void registerCommand(const std::string& command);

    template<typename Semi, template<typename ...> class L>
    std::shared_ptr<ParameterObject<NT>> createLayer(CommandScheme&, const std::string&);

    void fillArgs(ParameterObject<NT>*, CommandScheme&);
    
    std::unordered_map<std::string, CommandFactory> m_commandsFactories;
};

#include "router.tpp"

#endif // ROUTER_HPP