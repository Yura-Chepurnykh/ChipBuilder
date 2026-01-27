#ifndef SCENE_HPP
#define SCENE_HPP

#include <QGraphicsScene>
#include <QObject>
#include <QVector>
#include <cmath>
#include "layer.hpp"

class Scene : public QGraphicsScene
{
    Q_OBJECT

public:
    Scene() = default;
    Scene(qreal);

    void drawBackground(QPainter *painter, const QRectF &rect) override;

    void add(Layer*);
    void remove(Layer*);

private:
    qreal m_gap;
    QVector<Layer> m_layers;
};

#endif // SCENE_HPP
