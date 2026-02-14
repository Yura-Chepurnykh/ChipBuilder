#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <memory>
#include <vector>
#include <algorithm>
#include "layers.hpp"

class Context
{
public:
    void add(std::shared_ptr<AComponent>);
    void remove(unsigned int id);

private:
    std::vector<std::shared_ptr<AComponent>> m_components;
};

#endif // CONTEXT_HPP
