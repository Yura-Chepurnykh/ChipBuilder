#include "scene_view.hpp"

void SceneView::drawBackground(QPainter* painter, const QRectF& rect)
{
    qDebug() << "mousePressEvent";

    painter->fillRect(rect, QColor("#141414"));

    // if (!m_drawGrid)
    //     return;

    // temp gap
    constexpr qreal gap = 30;

    painter->setPen(QPen((QColor("#1c1c1c")), 2));

    for (auto left = std::floor(rect.left() / gap) * gap; left < rect.right(); left += gap)
    {
        painter->drawLine(left, rect.top(), left, rect.bottom());
    }

    for (auto top = std::floor(rect.top() / gap) * gap; top < rect.bottom(); top += gap)
    {
        painter->drawLine(rect.left(), top, rect.right(), top);
    }
}

void SceneView::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // qInfo() << "mousePressEvent";
    emit sceneClick(event->scenePos());
    event->accept();
}

void SceneView::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    emit sceneMouseMove(event->scenePos());
    // :w:qInfo() << "mouseMoveEvent";
    event->accept();
}

// void SceneView::keyPressEvent(QKeyEvent* event)
// {
// }

void SceneView::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    emit sceneMouseRelease(event->scenePos());
}

