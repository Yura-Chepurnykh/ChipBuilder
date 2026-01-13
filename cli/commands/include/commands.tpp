// commands.tpp

template<typename MOSType>
CreateMOSCommand::CreateMOSCommand(const std::string& name) : m_name(name) { }

template<typename MOSType>
void CreateMOSCommand::execute()
{

}

template <
    typename MOSType,
    typename Type,
    typename Dopant
>
CreateSubstrateCommand::CreateSubstrateCommand(
    MOSContext<MOSType>& context
) : m_context(context) { }

template<typename MOSType, typename Type, typename Dopant>
void CreateSubstrateCommand::execute() 
{

}

template <  
    typename MOSType,
    typename DiffusionType, 
    typename Type,
    typename Dopant
>
CreateDiffusionCommand::CreateDiffusionCommand(
    MOSContext<MOSType>& context
) : m_context(context) { }

template <
    typename MOSType,
    typename DiffusionType,
    typename Type,
    typename Dopant
>
void CreateDiffusionCommand::execute()
{

}

template<typename MOSType>
CreatePolySiliconCommand::CreatePolySiliconCommand(
    MOSContext<MOSType>& context
) : m_context(context) { }

template<typename MOSType>
void CreatePolySiliconCommand::execute() 
{

}
