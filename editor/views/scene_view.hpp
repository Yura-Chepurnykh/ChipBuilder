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
    QPointF startPoint;
    bool m_isDrawRect;

signals:
    void sceneClick(const QPointF&);
    void sceneMouseMove(const QPointF&);
    void sceneMouseRelease(const QPointF&);

signals:
    void MKeyPress();
    void DeleteKeyPress();

public slots:
    void handleDrawRectPreview();

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void keyPressEvent(QKeyEvent* event) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    // void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
};

#endif // SCENE_VIEW_H
