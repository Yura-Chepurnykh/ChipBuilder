#include "metal.hpp"

Metal::Metal()
{
    m_pen.setColor(Qt::magenta);
    m_pen.setWidth(50);
    m_pen.setStyle(Qt::SolidLine);
    m_pen.setJoinStyle(Qt::MiterJoin);
    m_pen.setCapStyle(Qt::FlatCap);
}

void Metal::add(std::shared_ptr<QPointF> p)
{
    m_path.push_back(p);
    update();
}

void Metal::remove(std::shared_ptr<QPointF> p)
{
    if (auto it = std::find(m_path.begin(), m_path.end(), p); it != m_path.end())
        m_path.erase(it);
    update();
}

QVector<std::shared_ptr<QPointF>> Metal::getPath()
{
    return m_path;
}

QRectF Metal::boundingRect() const
{
    return shape().boundingRect();
}

QPainterPath Metal::shape() const
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

void Metal::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(m_pen);

    for (int i = 1; i < m_path.size(); ++i)
        painter->drawLine(*m_path[i-1], *m_path[i]);
}











