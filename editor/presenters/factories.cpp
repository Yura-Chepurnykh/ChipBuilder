#include "factories.hpp"

QGraphicsItem* ViewFactory::create(IShape* shape, Style style, const QString& name)
{
    if (auto r = dynamic_cast<Rect*>(shape); r != nullptr)
    {
        auto view = new LayerView(toQRectF(*r), style, name);
        view->setPos(toQPointF(r->point));
        return view;
    }
    else if (auto m = dynamic_cast<PolygonShape*>(shape); m != nullptr)
    {
        auto view = new MetalView(toQSharedPolygon(*m), style);
        if (!m->m_points.empty())
            view->setPos(toQPointF(m->m_points[0]));
        return view;
    }
    return nullptr;
}
