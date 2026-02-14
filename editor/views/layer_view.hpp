#ifndef LAYER_VIEW_H
#define LAYER_VIEW_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QRectF>
#include <QHash>
#include <QColor>
#include <QPainter>
#include <QLabel>
#include "layer_model.hpp"

class LayerView : public QGraphicsItem
{
public:
    LayerView(const QRectF&, const Style&);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QRectF getRect() const { return m_rect; }
    Style getStyle() const { return m_style; }

    QRectF m_rect;
    Style m_style;
};

#endif // LAYER_VIEW_H
