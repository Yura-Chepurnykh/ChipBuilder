#include "factories.hpp"

QGraphicsItem* ViewFactory::create(IShape* shape, Style style, const QString& name)
{
    if (auto r = dynamic_cast<Rect*>(shape); r != nullptr)
    {
        return new LayerView(toQRectF(*r), style, name);
    }
    else if (auto m = dynamic_cast<PolygonShape*>(shape); m != nullptr)
    {
        return new MetalView(toQSharedPolygon(*m), style);
    }
    return nullptr;
}
