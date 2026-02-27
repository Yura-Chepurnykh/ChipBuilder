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
    enum ResizeDirection
    {
        None = 0,
        Left = 1 << 3,
        Top = 1 << 2,
        Right = 1 << 1,
        Bottom = 1 << 0
    };

    LayerView(const QRectF&, Style);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent*) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;

signals:
    void press(int);
    void moved(int, const QPointF&, const QPointF&);

public:
    int id;
    bool m_isDrag = false;
    int m_resizeDirection = 0;
    QPointF m_start, m_prevPos;
    QRectF m_rect;
    QColor m_baseColor;
    Style m_style;
};

#endif // LAYER_VIEW_H
