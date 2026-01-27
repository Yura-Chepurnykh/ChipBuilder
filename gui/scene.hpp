#ifndef SCENE_HPP
#define SCENE_HPP

#include <QGraphicsScene>
#include <QObject>
#include <QVector>
#include <QKeyEvent>
#include <cmath>
#include "layer.hpp"

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

private:
    bool drawGrid = false;
    qreal m_gap;
    QVector<Layer> m_layers;
};

#endif // SCENE_HPP
