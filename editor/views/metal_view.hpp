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
#include <QObject>
#include "layer_model.hpp"

class MetalView : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    using QSharedPolygon = QVector<std::shared_ptr<QPointF>>;

    MetalView(QSharedPolygon, Style, int thickness = 2);
    ~MetalView() = default;

    int id;

    QVector<std::shared_ptr<QPointF>> getPath() const { return m_path; }
    void setDRCViolated(bool violated) { m_drcViolated = violated; update(); }

    void setThickness(int thickness);
    int thickness() const { return m_thickness; }

signals:
    void geometryChanged(int); // Emitted when points move
    void thicknessChanged(int, int); // id, newThickness

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    Style m_style;
    QVector<std::shared_ptr<QPointF>> m_path;
    int m_thickness = 2;
    bool m_drcViolated = false;

    int m_draggedSegmentIdx = -1; // -1 if not dragging a segment
    int m_draggedPointIdx = -1;   // -1 if not dragging a point
    bool m_isResizing = false;
    QPointF m_lastMousePos;
    QPointF m_pressPos;
    QVector<QPointF> m_initialPoints;
};

#endif // METAL_VIEW_H
