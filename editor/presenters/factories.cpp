#include "factories.hpp"

QGraphicsItem* ViewFactory::create(IShape* shape, Style style)
{
    if (auto r = dynamic_cast<Rect*>(shape); r != nullptr)
    {
        return new LayerView(toQRectF(*r), style);
    }
    else if (auto m = dynamic_cast<Polygon*>(shape); m != nullptr)
    {
        return new MetalView(toQSharedPolygon(*m), style);
    }
    return nullptr;
}
