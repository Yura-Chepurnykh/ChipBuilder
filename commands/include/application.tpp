// application.tpp

static Application& Application::instance()
{
    static Application application;
    return application;
}
    
void Application::add(std::unique_ptr<IContext> context)
{
    m_contexts.push_back(std::move(context));
}
