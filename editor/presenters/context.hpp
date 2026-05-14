#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "layers.hpp"
#include "drc.hpp"

class Context
{
public:
    Context() : m_rules(DRCRuleSet::defaultRules()) {}
    // void add(std::shared_ptr<AComponent>);
    // void remove(unsigned int id);
    std::unordered_map<unsigned int, unsigned int> m_modelToView;
    std::unordered_map<unsigned int, unsigned int> m_viewToModel;

// private:
    CircuitLayout m_layout;
    DRCRuleSet m_rules;
    std::vector<DRCViolation> m_violations;
};

#endif // CONTEXT_HPP
