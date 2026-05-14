#include "factories.hpp"
#include "layers.hpp"

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

QGraphicsItem* ViewFactory::create(AComponent* component, Style style)
{
    if (!component) return nullptr;
    
    if (auto metal = dynamic_cast<Metal1*>(component)) {
        if (auto poly = dynamic_cast<PolygonShape*>(metal->getShape())) {
            auto view = new MetalView(toQSharedPolygon(*poly), style, metal->thickness);
            if (!poly->m_points.empty())
                view->setPos(toQPointF(poly->m_points[0]));
            return view;
        }
    }
    
    return create(component->getShape(), style, QString::fromStdString(component->name()));
}
