// commands.tpp

template<typename S>
CreateMOSCommand<S>::CreateMOSCommand(std::shared_ptr<IParameterObject> params) : m_params(params) { }

template<typename S>
void CreateMOSCommand<S>::execute() { std::cout << "CreateMOSCommand" << std::endl; }

template <typename S,typename D>
CreateSubstrateCommand<S, D>::CreateSubstrateCommand(std::shared_ptr<IParameterObject> params) : m_params(params) { }

template<typename S, typename D>
void CreateSubstrateCommand<S, D>::execute() { std::cout << "CreateSubstrateCommand" << std::endl; }

template<typename S, typename Diffusion, typename D>
CreateDiffusionCommand<S, Diffusion, D>::CreateDiffusionCommand(std::shared_ptr<IParameterObject> params) : m_params(params) { }

template<typename S, typename Diffusion, typename D>
void CreateDiffusionCommand<S, Diffusion, D>::execute() { std::cout << "CreateDiffusionCommand" << std::endl; }

template<typename S>
CreatePolyCommand<S>::CreatePolyCommand(std::shared_ptr<IParameterObject> params) : m_params(params) { }

template<typename S>
void CreatePolyCommand<S>::execute() { std::cout << "CreatePolyCommand" << std::endl; }

template<typename S>
CreateOxideCommand<S>::CreateOxideCommand(std::shared_ptr<IParameterObject> params) : m_params(params) { }

template<typename S>
void CreateOxideCommand<S>::execute() { std::cout << "CreateOxideCommand" << std::endl; }

