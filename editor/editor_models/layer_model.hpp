#ifndef LAYER_MODEL_HPP
#define LAYER_MODEL_HPP

#include <unordered_map>
#include <typeindex>
#include <string>
#include <type_traits>
#include <optional>
#include <QColor>
#include <QPen>
#include "layers.hpp"

enum class State
{
    Normal,
    Selected,
    Hover
};

struct Style
{
    QColor background;
    QPen pen;
};

class LayerModel
{
public:
    LayerModel()
    {
        registerStyle(typeid(Substrate<NType>), Style{ QColor(Qt::red),     QPen(Qt::black, 1)});
        registerStyle(typeid(Substrate<PType>), Style{ QColor(Qt::cyan),    QPen(Qt::black, 1)});
        registerStyle(typeid(Diffusion<NType>), Style{ QColor(Qt::blue),    QPen(Qt::black, 1)});
        registerStyle(typeid(Diffusion<PType>), Style{ QColor(Qt::gray),    QPen(Qt::black, 1)});
        registerStyle(typeid(Oxide),            Style{ QColor(Qt::green),   QPen(Qt::black, 1)});
        registerStyle(typeid(PolySilicon),      Style{ QColor(Qt::darkRed), QPen(Qt::black, 1)});
    }

    void registerStyle(std::type_index idx, const Style& value)
    {
        m_styles[idx] = value;
    }

    std::optional<Style> getStyle(std::type_index idx)
    {
        if (auto it = m_styles.find(idx); it != m_styles.end())
        {
            return it->second;
        }

        return std::nullopt;
    }

private:
    std::unordered_map<std::type_index, Style> m_styles;
};

// TODO: implement a version with StyleKey
// enum class State
// {
//     Normal,
//     Selected,
//     Hover
// };

// struct StyleKey
// {
//     std::type_index typeIdx;
//     State state;

//     bool operator==(const StyleKey& other) const
//     {
//         return state == other.state && typeIdx == other.typeIdx;
//     }
// };

// struct Style
// {
//     QColor background;
//     QPen pen;
// };

// struct StyleKeyHash
// {
//     size_t operator()(const StyleKey& key) const
//     {
//         auto h1 = std::hash<std::type_index>{}(key.typeIdx);
//         auto h2 = std::hash<std::underlying_type_t<State>>{}
//             (static_cast<std::underlying_type_t<State>>(key.state));

//         return h1 ^ (h2 << 1);
//     }
// };

// class LayerModel
// {
// public:
//     LayerModel()
//     {
//         registerStyle(typeid(Substrate<NType>), Style{ QColor(Qt::blue),    QPen(Qt::red, 2)});
//         registerStyle(typeid(Substrate<PType>), Style{ QColor(Qt::cyan),    QPen(Qt::red, 2)});
//         registerStyle(typeid(Diffusion<NType>), Style{ QColor(Qt::blue),    QPen(Qt::red, 2)});
//         registerStyle(typeid(Diffusion<PType>), Style{ QColor(Qt::gray),    QPen(Qt::red, 2)});
//         registerStyle(typeid(Oxide),            Style{ QColor(Qt::green),   QPen(Qt::red, 2)});
//         registerStyle(typeid(PolySilicon),      Style{ QColor(Qt::darkRed), QPen(Qt::red, 2)});
//     }

//     void registerStyle(std::type_index idx, const Style& value, State state = State::Normal)
//     {
//         m_styles[StyleKey{ typeid(idx), state }] = value;
//     }

//     std::optional<Style> getStyle(std::type_index idx, State state = State::Normal)
//     {
//         if (auto it = m_styles.find({typeid(idx), state}); it != m_styles.end())
//         {
//             return it->second;
//         }

//         return std::nullopt;
//     }

// private:
//     std::unordered_map<StyleKey, Style, StyleKeyHash> m_styles;
// };

#endif // LAYER_MODEL_HPP
