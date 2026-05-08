#ifndef SCENE_VIEW_H
#define SCENE_VIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QObject>
#include <QVector>
#include <QKeyEvent>
#include <QGraphicsSceneWheelEvent>
#include <QAction>
#include <QGraphicsRectItem>
#include <QDebug>
#include <cmath>
#include <utility>
#include <optional>
#include <memory>
#include "toolbar.hpp"
#include "layer_view.hpp"
#include "metal_view.hpp"

class Toolbar;

class SceneView : public QGraphicsScene
{
    Q_OBJECT

public:
    SceneView() = default;

    QGraphicsRectItem* preview = nullptr;
    QGraphicsPathItem* polygonPreview = nullptr;
    QGraphicsRectItem* m_cursorPreview = nullptr;
    QGraphicsPathItem* lassoPreview = nullptr;
    QPointF startPoint;
    bool m_isDrawRect;
    bool m_isDrawPolygon = false;
    bool m_isRectSelection;
    bool m_isLassoSelection;
    QPainterPath m_lassoPath;
    QVector<QPointF> m_polygonPoints;

signals:
    void sceneClick(const QPointF&);
    void sceneMouseMove(const QPointF&);
    void sceneMouseRelease(const QPointF&);
    void sceneMouseDoubleClick(const QPointF&);

signals:
    void mKeyPress();
    void deleteKeyPress();
    void undoPress();
    void redoPress();

public slots:
    void handleDrawRectPreview();
    void handleDrawPolygonPreview();

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void keyPressEvent(QKeyEvent* event) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};

#endif // SCENE_VIEW_H
