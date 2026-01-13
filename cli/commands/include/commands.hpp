#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
#include "layers.hpp"

class ICommand 
{
public:
    virtual void execute() = 0;
    virtual ~ICommand() = default;
};

template<typename MOSType>
class CreateMOSCommand : public ICommand 
{
public:
    CreateMOSCommand(const std::string& name);
    void execute() override;

private:
    std::string m_name;
};

template<typename MOSType, typename Type, typename Dopant>
class CreateSubstrateCommand : public ICommand
{
public:
    CreateSubstrateCommand(MOSContext<MOSType>&);
    void execute() override;

private:
    MOSContext<MOSType>& m_context;
};

template<typename MOSType, typename DiffusionType, typename Type, typename Dopant>
class CreateDiffusionCommand : public ICommand 
{
public:
    CreateDiffusionCommand(MOSContext<MOSType>&);
    void execute() override;

private:
    MOSContext<MOSType>& m_context;
};

template<typename MOSType>
class CreatePolySiliconCommand : public ICommand 
{
public:
    CreatePolySiliconCommand(MOSContext<MOSType>&);
    void execute() override;

private:
    MOSContext<MOSType>& m_context;
};

#include "commands.tpp"

#endif // COMMANDS_HPP
