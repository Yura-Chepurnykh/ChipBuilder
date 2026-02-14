#include "mappers.hpp"

Point toPoint(const QPointF& p)
{
    int x = static_cast<int>(p.x());
    int y = static_cast<int>(p.y());
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
    QPointF p = toQPointF(r.point);
    qreal w = static_cast<qreal>(r.width);
    qreal h = static_cast<qreal>(r.height);
    return QRectF(p.x(), p.y(), w, h);
}
