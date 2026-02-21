#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "layers.hpp"

class Context
{
public:
    Context() = default;
    // void add(std::shared_ptr<AComponent>);
    // void remove(unsigned int id);
    std::unordered_map<unsigned int, unsigned int> m_modelToView;
    std::unordered_map<unsigned int, unsigned int> m_viewToModel;

// private:
    CircuitLayout m_layout;
};

#endif // CONTEXT_HPP
