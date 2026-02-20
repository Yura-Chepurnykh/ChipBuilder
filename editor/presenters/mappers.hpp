#ifndef MAPPERS_H
#define MAPPERS_H

#include <QPointF>
#include <QRectF>
#include <QVector>
#include <memory>
#include "geometry.hpp"
#include "id_generator.hpp"

Point toPoint(const QPointF&);
QPointF toQPointF(const Point&);
QRectF toQRectF(const Rect&);
QVector<std::shared_ptr<QPointF>> toQSharedPolygon(const Polygon&);

#endif // MAPPERS_H
