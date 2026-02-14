#include "context.hpp"

void Context::add(std::shared_ptr<AComponent> component)
{
    m_components.push_back(component);
}

void Context::remove(unsigned int id)
{
    if (auto it = std::ranges::find_if(m_components, [&](const std::shared_ptr<AComponent>& component){
        return id == component->id;
    }); it != m_components.end())
    {
        m_components.erase(it);
    }
}
