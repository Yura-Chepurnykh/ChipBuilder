#include "scene_view.hpp"
#include "layer_view.hpp"

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

void SceneView::keyPressEvent(QKeyEvent* event)
{
    switch(event->key())
    {
    case Qt::Key_M:
        emit MKeyPress();
    }

    event->accept();
}

void SceneView::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "SceneView::mousePressEvent\n");

    if (event->isAccepted())
        return;

    if (m_isDrawRect)
    {
        startPoint = event->scenePos();
        preview = addRect(QRectF(startPoint, startPoint), QPen(QColor(Qt::gray), 1));
        emit sceneClick(event->scenePos());
    }

    QGraphicsScene::mousePressEvent(event);
    // event->accept();
}

void SceneView::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "SceneView::mouseMoveEvent\n");

    // if (event->isAccepted())
    //     return;

    if (preview != nullptr & m_isDrawRect)
    {
        QRectF rect(startPoint, event->scenePos());
        rect = rect.normalized();
        preview->setRect(rect);
        emit sceneMouseMove(event->scenePos());
    }

    QGraphicsScene::mouseMoveEvent(event);
    // event->accept();
}

void SceneView::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "SceneView::mouseReleaseEvent\n");

    // if (event->isAccepted())
    //     return;

    if (preview != nullptr & m_isDrawRect)
    {
        removeItem(preview);
        delete preview;
        preview = nullptr;
        m_isDrawRect = false;
    }

    emit sceneMouseRelease(event->scenePos());
    QGraphicsScene::mouseReleaseEvent(event);
}

void SceneView::handleDrawRectPreview() { m_isDrawRect = true; }


