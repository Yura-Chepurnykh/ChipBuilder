#ifndef LAYER_HPP
#define LAYER_HPP

#include <QGraphicsItem>
#include <QRectF>
#include <QHash>
#include <QColor>
#include <QPainter>

class Layer : public QGraphicsItem
{
public:
    Layer(const QRectF&, const QColor&);

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    // getters
    QRectF getRect();
    QRectF const getRect() const;

private:
    QColor m_color;
    QRectF m_rect;
};

#endif // LAYER_HPP
