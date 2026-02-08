#ifndef LAYER_MODEL_HPP
#define LAYER_MODEL_HPP

#include <unordered_map>
#include <typeindex>
#include <string>
#include <type_traits>
#include <optional>
#include "layers.hpp"

enum class State 
{
    Normal,
    Selected,
    Hover
};

struct StyleKey
{
    std::type_index typeIdx;
    State state;

    bool operator==(const StyleKey& other) const 
    {
        return state == other.state && typeIdx == other.typeIdx;
    }
};

struct StyleKeyHash
{
    size_t operator()(const StyleKey& key) const 
    {
        auto h1 = std::hash<std::type_index>{}(key.typeIdx);
        auto h2 = std::hash<std::underlying_type_t<State>>{}
            (static_cast<std::underlying_type_t<State>>(key.state));

        return h1 ^ (h2 << 1);
    }
};

class LayerModel
{
public:
    using CSS = std::string;

    template<typename LayerType>
    void registerStyle(State state, const std::string& cssStyle) 
    {
        static_assert(std::is_base_of<Layer, LayerType>, "LayerType must be derived from Layer");
        m_styles[{ typeid(LayerType), state }] = cssStyle;
    }

    template<typename LayerType>
    std::optional<std::string> getStyle(State state)
    {
        static_assert(std::is_base_of<Layer, LayerType>, "LayerType must be derived from Layer");

        if (auto it = m_styles.find({typeid(LayerType), state}); it != m_styles.end())
        {
            return it->second;
        }

        return std::nullopt;
    }

private:
    std::unordered_map<StyleKey, CSS, StyleKeyHash> m_styles;
};

#endif // LAYER_MODEL_HPP
