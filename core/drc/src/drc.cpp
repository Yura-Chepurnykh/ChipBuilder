#include "drc.hpp"

Rule::Rule() : m_next(nullptr) { }

void Rule::setNext(std::shared_ptr<Rule> rule) 
{
    m_next = rule;
}

void Rule::handle(ParameterObject& param)
{
    if (m_next)
        m_next->handle(param);
}

MinimumWidth::MinimumWidth(int minWidth) : m_minWidth(minWidth) { }

void MinimumWidth::handle(ParameterObject& param)
{
    if (auto p = dynamic_cast<const MinimumWidthParameterObject*>(&param))
    {
        if (m_minWidth >= p->width)
            throw std::runtime_error("Illegal width");
        else 
            Rule::handle(param);
    }
    else 
        throw std::runtime_error("Parameter is not MinimumWidthParameterObject");
}

MinimumSpacing::MinimumSpacing(int minSpace) noexcept : m_minSpace(minSpace) { }

void MinimumSpacing::handle(ParameterObject& param)
{
    if (auto p = dynamic_cast<const MinimumSpacingParameterObject*>(&param))
    {
        if (distance(p->shape1, p->shape2) >= m_minSpace)
            throw std::runtime_error("Space is less");
        else 
            Rule::handle(param);
    }
    else 
        throw std::runtime_error("Parameter is not MinimumSpacingParameterObject");
}

MinimumArea::MinimumArea(int minArea) noexcept : m_minArea(minArea) { }

void MinimumArea::handle(ParameterObject& param)
{
    if (auto p = dynamic_cast<const MinimumAreaParameterObject*>(&param))
    {
        if (p->area >= m_minArea)
            throw std::runtime_error("Area is small");
        else 
            Rule::handle(param);
    }
    else 
        throw std::runtime_error("Parameter is not MinimumAreaParameterObject");
}