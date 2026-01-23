#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
#include <memory>

#include "layers.hpp"
#include "circuits.hpp"
#include "context.hpp"
#include "parameter_object.hpp"

class ICommand 
{
public:
    virtual void execute() = 0;
    virtual ~ICommand() = default;
};

template<typename S>
class CreateMOSCommand : public ICommand 
{
public:
    CreateMOSCommand(std::shared_ptr<IParameterObject>);
    void execute() override;

private:
    std::shared_ptr<IParameterObject> m_params;
};

template<typename S, typename D>
class CreateSubstrateCommand : public ICommand
{
public:
    CreateSubstrateCommand(std::shared_ptr<IParameterObject>);
    void execute() override;

private:
    std::shared_ptr<IParameterObject> m_params;
};

template<typename S, typename Diffusion, typename D>
class CreateDiffusionCommand : public ICommand 
{
public:
    CreateDiffusionCommand(std::shared_ptr<IParameterObject>);
    void execute() override;

private:
    std::shared_ptr<IParameterObject> m_params;
};

template<typename S>
class CreatePolyCommand : public ICommand 
{
public:
    CreatePolyCommand(std::shared_ptr<IParameterObject>);
    void execute() override;

private:
    std::shared_ptr<IParameterObject> m_params;
};

template<typename S>
class CreateOxideCommand : public ICommand 
{
public:
    CreateOxideCommand(std::shared_ptr<IParameterObject>);
    void execute() override;

private:
    std::shared_ptr<IParameterObject> m_params;
};

#include "commands.tpp"

#endif // COMMANDS_HPP
