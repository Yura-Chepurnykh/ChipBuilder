#ifndef FACTORIES_HPP
#define FACTORIES_HPP

#include <QGraphicsItem>
#include "geometry.hpp"
#include "layer_model.hpp"
#include "layer_view.hpp"
#include "metal_view.hpp"
#include "mappers.hpp"

// pattern factory method
class ViewFactory
{
public:
    static QGraphicsItem* create(IShape* shape, Style style, const QString& name = "");
};

#endif // FACTORIES_HPP
