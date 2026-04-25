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

class StyleModel
{
public:
    StyleModel()
    {
        registerStyle(typeid(Active),  Style{ QColor(Qt::green),       QPen(Qt::black, 1)});
        registerStyle(typeid(Poly),    Style{ QColor(Qt::red),         QPen(Qt::black, 1)});
        registerStyle(typeid(NWell),   Style{ QColor(Qt::yellow),      QPen(Qt::black, 1)});
        registerStyle(typeid(PWell),   Style{ QColor(Qt::cyan),        QPen(Qt::black, 1)});
        registerStyle(typeid(Metal1),  Style{ QColor(Qt::blue),        QPen(Qt::black, 1)});
        registerStyle(typeid(Via),     Style{ QColor(Qt::white),       QPen(Qt::black, 1)});
        registerStyle(typeid(Contact), Style{ QColor(Qt::gray),        QPen(Qt::black, 1)});
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

#endif // LAYER_MODEL_HPP
