// commands.tpp

template<typename S>
CreateMOSCommand<S>::CreateMOSCommand(const std::string& name) : m_name(name) { }

template<typename S>
void CreateMOSCommand<S>::execute() { }

template <typename S,typename D>
CreateSubstrateCommand<S, D>::CreateSubstrateCommand(MOSContext<S, D>& context) : m_context(context) { }

template<typename S, typename D>
void CreateSubstrateCommand<S, D>::execute() { }

template<typename S, typename Diffusion, typename D>
CreateDiffusionCommand<S, Diffusion, D>::CreateDiffusionCommand(MOSContext<S>& context) : m_context(context) { }

template<typename S, typename Diffusion, typename D>
void CreateDiffusionCommand<S, Diffusion, D>::execute() { }

template<typename S>
CreatePolyCommand<S>::CreatePolyCommand(MOSContext<S>& context) : m_context(context) { }

template<typename S>
void CreatePolyCommand<S>::execute() { }

template<typename S>
CreateOxideCommand<S>::CreateOxideCommand(MOSContext<S>& context) : m_context(context) { }

template<typename S>
void CreateOxideCommand<S>::execute() { }

