#ifndef MAPPERS_H
#define MAPPERS_H

#include <QPointF>
#include <QRectF>
#include "geometry.hpp"
#include "id_generator.hpp"

Point toPoint(const QPointF& p);
QPointF toQPointF(const Point& p);
QRectF toQRectF(const Rect& r);

#endif // MAPPERS_H
