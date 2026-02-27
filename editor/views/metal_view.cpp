#include "metal_view.hpp"
#include "id_generator.hpp"

MetalView::MetalView(QSharedPolygon p, Style s) : id(IdGenerator::generate()), m_path(p), m_style(s)
{
    m_style.pen.setColor(Qt::magenta);
    m_style.pen.setWidth(50);
    m_style.pen.setStyle(Qt::SolidLine);
    m_style.pen.setJoinStyle(Qt::MiterJoin);
    m_style.pen.setCapStyle(Qt::FlatCap);
}

QRectF MetalView::boundingRect() const
{
    return shape().boundingRect();
}

QPainterPath MetalView::shape() const
{
    QPainterPath path;

    if (m_path.isEmpty()) return path;

    path.moveTo(*m_path[0]);

    for (int i = 1; i < m_path.size(); ++i)
        path.lineTo(*m_path[i]);

    QPainterPathStroker stroker;
    stroker.setCapStyle(m_style.pen.capStyle());
    stroker.setJoinStyle(m_style.pen.joinStyle());
    stroker.setWidth(m_style.pen.width());

    return stroker.createStroke(path);
}

void MetalView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(m_style.pen);

    for (int i = 1; i < m_path.size(); ++i)
        painter->drawLine(*m_path[i-1], *m_path[i]);
}
