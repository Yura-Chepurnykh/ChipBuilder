#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
#include "layers.hpp"
#include "circuits.hpp"
#include "context.hpp"

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
    CreateMOSCommand(const std::string& name);
    void execute() override;

private:
    std::string m_name;
};

template<typename S, typename D>
class CreateSubstrateCommand : public ICommand
{
public:
    CreateSubstrateCommand(MOSContext<S, D>&);
    void execute() override;

private:
    MOSContext<S>& m_context;
};

template<typename S, typename Diffusion, typename D>
class CreateDiffusionCommand : public ICommand 
{
public:
    CreateDiffusionCommand(MOSContext<S>&);
    void execute() override;

private:
    MOSContext<S>& m_context;
};

template<typename S>
class CreatePolyCommand : public ICommand 
{
public:
    CreatePolyCommand(MOSContext<S>&);
    void execute() override;

private:
    MOSContext<S>& m_context;
};

template<typename S>
class CreateOxideCommand : public ICommand 
{
public:
    CreateOxideCommand(MOSContext<S>&);
    void execute() override;

private:
    MOSContext<S>& m_context;
};

#include "commands.tpp"

#endif // COMMANDS_HPP
