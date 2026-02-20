#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <memory>
#include <vector>
#include <algorithm>
#include "layers.hpp"

class Context
{
public:
    Context() = default;
    // void add(std::shared_ptr<AComponent>);
    // void remove(unsigned int id);

// private:
    CircuitLayout m_layout;
};

#endif // CONTEXT_HPP
