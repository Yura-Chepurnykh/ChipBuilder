#include "layer_view.hpp"

LayerView::LayerView(const QRectF& r, Style s) : m_rect(r), m_style(s) { }

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

void LayerView::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "LayerView::mousePressEvent\n");
    m_isDrag = true;
    m_start = event->pos();
    event->accept();
    //QGraphicsItem::mousePressEvent(event);
}

void LayerView::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "LayerView::mouseMoveEvent\n");
    if (m_isDrag)
    {
        setPos(mapToScene(event->pos() - m_start));
    }
    event->accept();
    //QGraphicsItem::mouseMoveEvent(event);
}

void LayerView::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "LayerView::mouseReleaseEvent\n");
    m_isDrag = false;
    event->accept();
    //QGraphicsItem::mouseReleaseEvent(event);
}

