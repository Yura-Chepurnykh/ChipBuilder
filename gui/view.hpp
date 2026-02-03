#ifndef VIEW_HPP
#define VIEW_HPP

#include <QGraphicsView>
#include <QWheelEvent>

#include "scene.hpp"

class View : public QGraphicsView
{
public:
    View(Scene* scene);
    void wheelEvent(QWheelEvent* event) override;

private:
    Scene* m_scene;
};

#endif // VIEW_HPP
