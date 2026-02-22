#ifndef LAYER_VIEW_H
#define LAYER_VIEW_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QRectF>
#include <QHash>
#include <QColor>
#include <QObject>
#include <QPainter>
#include <QLabel>
#include "layer_model.hpp"
#include "id_generator.hpp"

class LayerView : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    LayerView(const QRectF&, Style);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QRectF getRect() const { return m_rect; }
    Style getStyle() const { return m_style; }

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent*) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;

signals:
    void press(int);

public:
    QColor m_baseColor;
    unsigned int id;
    QPointF m_start;
    bool m_isDrag = false;
    QRectF m_rect;
    Style m_style;
};

#endif // LAYER_VIEW_H
