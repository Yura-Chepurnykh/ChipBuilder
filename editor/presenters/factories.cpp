#include "factories.hpp"

QGraphicsItem* ViewFactory::create(std::unique_ptr<IShape> shape, Style style)
{
    if (auto r = dynamic_cast<Rect*>(shape.get()); r != nullptr)
    {
        return new LayerView(toQRectF(*r), style);
    }
    else if (auto m = dynamic_cast<Polygon*>(shape.get()); m != nullptr)
    {
        return new MetalView(toQSharedPolygon(*m), style);
    }
    return nullptr;
}
