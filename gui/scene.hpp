#ifndef SCENE_HPP
#define SCENE_HPP

#include <QGraphicsScene>
#include <QObject>
#include <QVector>
#include "layer.hpp"

class Scene : public QGraphicsScene
{
    Q_OBJECT

public:
    Scene() = default;

    void add(Layer*);
    void remove(Layer*);

private:
    qreal gap;
    QVector<Layer> m_layers;
};

#endif // SCENE_HPP
