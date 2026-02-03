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

class MetalView : public QGraphicsItem
{
public:
    MetalView();
    ~MetalView() = default;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

private:
    QPen m_pen;
    QVector<std::shared_ptr<QPointF>> m_path;
};

#endif // METAL_VIEW_H
