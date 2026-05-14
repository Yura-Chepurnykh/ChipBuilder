#include "mappers.hpp"

Point toPoint(const QPointF& p)
{
    constexpr int gap = 30;
    int x = static_cast<int>(std::round(p.x() / gap)) * gap;
    int y = static_cast<int>(std::round(p.y() / gap)) * gap;
    return Point(IdGenerator::generate(), x, y);
}

QPointF toQPointF(const Point& p)
{
    qreal x = static_cast<qreal>(p.x);
    qreal y = static_cast<qreal>(p.y);
    return QPointF(x, y);
}

QRectF toQRectF(const Rect& r)
{
    qreal w = static_cast<qreal>(r.width);
    qreal h = static_cast<qreal>(r.height);
    return QRectF(0, 0, w, h);
}

QVector<std::shared_ptr<QPointF>> toQSharedPolygon(const PolygonShape& polygon)
{
    QVector<std::shared_ptr<QPointF>> sharedPolygon;
    if (polygon.m_points.empty()) return sharedPolygon;

    QPointF first = toQPointF(polygon.m_points[0]);

    for (int i = 0; i < polygon.m_points.size(); ++i)
    {
        QPointF qPointF = toQPointF(polygon.m_points[i]);
        // Make local relative to the first point
        auto sharedQPointF = std::make_shared<QPointF>(qPointF - first);
        sharedPolygon.push_back(sharedQPointF);
    }

    return sharedPolygon;
}
