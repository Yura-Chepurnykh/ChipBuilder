#include "layer_view.hpp"

LayerView::LayerView(const QRectF& r, const Style& s) : m_rect(r), m_style(s) { }

QRectF LayerView::boundingRect() const
{
    return m_rect;
}

void LayerView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(QColor(m_style.background));
    painter->setPen(m_style.pen);
    painter->drawRect(m_rect);
}
