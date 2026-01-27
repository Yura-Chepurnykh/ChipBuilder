#include "view.hpp"

View::View(Scene* scene) : m_scene(scene)
{
    setMouseTracking(true);
}

void View::wheelEvent(QWheelEvent* event)
{
    event->angleDelta().y() > 0 ? scale(1.1, 1.1) : scale(0.9, 0.9);
}
