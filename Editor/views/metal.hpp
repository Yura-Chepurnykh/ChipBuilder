#ifndef METAL_HPP
#define METAL_HPP

#include <QGraphicsItem>
#include <QVector>
#include <QPointF>
#include <QPen>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QPainter>
#include <memory>
#include <iterator>

class Metal : public QGraphicsItem
{
public:
    Metal();
    ~Metal() = default;

    void add(std::shared_ptr<QPointF>);
    void remove(std::shared_ptr<QPointF>);

    QVector<std::shared_ptr<QPointF>> getPath();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

private:
    QPen m_pen;
    QVector<std::shared_ptr<QPointF>> m_path;
};

#endif // METAL_HPP
