#ifndef METAL_VIEW_H
#define METAL_VIEW_H

#include <QGraphicsItem>
#include <QVector>
#include <QPointF>
#include <QPen>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QPainter>
#include <memory>
#include <iterator>
#include "layer_model.hpp"

class MetalView : public QGraphicsItem
{
public:
    using QSharedPolygon = QVector<std::shared_ptr<QPointF>>;

    MetalView(QSharedPolygon, Style);
    ~MetalView() = default;

    int id;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

private:
    Style m_style;
    QVector<std::shared_ptr<QPointF>> m_path;
};

#endif // METAL_VIEW_H
