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
#include "layer.hpp"
#include "metal_view.hpp"

class ToolBar;

class SceneView : public QGraphicsScene
{
    Q_OBJECT

public:
    struct Options
    {
        bool m_layer = false;
        bool m_metal = false;
    };

    SceneView() = default;
    SceneView(qreal);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    qreal m_gap;
    Options m_options;
    bool m_drawGrid = false;
};

#endif // SCENE_VIEW_H
