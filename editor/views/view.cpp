#include "view.hpp"

View::View(SceneView* scene) : m_scene(scene)
{
    //setMouseTracking(true);
}

void View::wheelEvent(QWheelEvent* event)
{
    event->angleDelta().y() > 0 ? scale(1.1, 1.1) : scale(0.9, 0.9);
}

void View::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton)
    {
        setDragMode(QGraphicsView::ScrollHandDrag);
        // Simulate a left button press to start dragging immediately
        QMouseEvent fakeEvent(event->type(), event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());
        QGraphicsView::mousePressEvent(&fakeEvent);
    }
    else
    {
        QGraphicsView::mousePressEvent(event);
    }
}

void View::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton)
    {
        setDragMode(QGraphicsView::NoDrag);
    }
    QGraphicsView::mouseReleaseEvent(event);
}
