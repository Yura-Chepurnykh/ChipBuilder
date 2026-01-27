#ifndef SCENE_HPP
#define SCENE_HPP

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QObject>
#include <QVector>
#include <QKeyEvent>
#include <QGraphicsSceneWheelEvent>
#include <QAction>
#include <QDebug>
#include <cmath>
#include <utility>
#include <optional>
#include "toolbar.hpp"
#include "layer.hpp"

class ToolBar;

class Scene : public QGraphicsScene
{
    Q_OBJECT

public:
    Scene() = default;
    Scene(qreal);

    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void keyPressEvent(QKeyEvent* event) override;

    void add(Layer*);
    void remove(Layer*);

public slots:
    void nSubstrate();
    void pSubstrate();
    void nSource();
    void pSource();
    void nDrain();
    void pDrain();
    void oxide();
    void polysilicon();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

private:
    std::optional<Layer::Type> currentLayer;
    ToolBar* m_toolbar;
    bool drawGrid = false;
    qreal m_gap;
    QVector<Layer> m_layers;
};

#endif // SCENE_HPP
