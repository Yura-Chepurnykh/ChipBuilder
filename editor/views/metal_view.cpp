#include "metal_view.hpp"

MetalView::MetalView()
{
    m_pen.setColor(Qt::magenta);
    m_pen.setWidth(50);
    m_pen.setStyle(Qt::SolidLine);
    m_pen.setJoinStyle(Qt::MiterJoin);
    m_pen.setCapStyle(Qt::FlatCap);
}

QRectF MetalView::boundingRect() const
{
    return shape().boundingRect();
}

QPainterPath MetalView::shape() const
{
    QPainterPath path;

    if (path.isEmpty()) return path;

    path.moveTo(*m_path[0]);

    for (int i = 1; i < m_path.size(); ++i)
        path.lineTo(*m_path[i]);

    QPainterPathStroker stroker;
    stroker.setCapStyle(m_pen.capStyle());
    stroker.setJoinStyle(m_pen.joinStyle());
    stroker.setWidth(m_pen.width());

    return stroker.createStroke(path);
}

void MetalView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(m_pen);

    for (int i = 1; i < m_path.size(); ++i)
        painter->drawLine(*m_path[i-1], *m_path[i]);
}
