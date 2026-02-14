#ifndef VIEW_HPP
#define VIEW_HPP

#include <QGraphicsView>
#include <QWheelEvent>
#include <QColor>

#include "scene_view.hpp"

class View : public QGraphicsView
{
public:
    View(SceneView* scene);
    void wheelEvent(QWheelEvent* event) override;

private:
    SceneView* m_scene;
};

#endif // VIEW_HPP
