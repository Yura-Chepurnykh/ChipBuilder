#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

class Router 
{
public:
    using Factory = std::function<std::unique_ptr<Command>(const ParameterObject&)>;
    
    Router();

    template<typename CommandType>
    void registerCommand(const std::string&);

private:
    std::unordered_map<std::string, Factory> factoriesMap;
};

#endif // ROUTER_HPP
