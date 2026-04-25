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
        painter->drawLine(left, rect.top(), left, rect.bottom());

    for (auto top = std::floor(rect.top() / gap) * gap; top < rect.bottom(); top += gap)
        painter->drawLine(rect.left(), top, rect.right(), top);
}

void SceneView::keyPressEvent(QKeyEvent* event)
{
    switch(event->key())
    {
        case Qt::Key_M:
        {
            emit mKeyPress();
            break;
        }
        case Qt::Key_Delete:
        {
            emit deleteKeyPress();
            break;
        }
        case Qt::Key_Z:
        {
            emit undoPress();
            break;
        }
        case Qt::Key_X:
        {
            emit redoPress();
            break;
        }
    }

    event->accept();
}

void SceneView::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "SceneView::mousePressEvent\n");

    if (event->isAccepted())
        return;

    if (m_isDrawRect || m_isRectSelection)
    {
        constexpr int gap = 30;
        qreal snappedX = std::round(event->scenePos().x() / gap) * gap;
        qreal snappedY = std::round(event->scenePos().y() / gap) * gap;
        startPoint = QPointF(snappedX, snappedY);
        preview = addRect(QRectF(startPoint, startPoint), QPen(QColor(Qt::gray), 1, Qt::DashLine));
    }
    else if (m_isLassoSelection)
    {
        m_lassoPath = QPainterPath();
        m_lassoPath.moveTo(event->scenePos());
        lassoPreview = addPath(m_lassoPath, QPen(QColor(Qt::blue), 1, Qt::DashLine));
    }

    emit sceneClick(event->scenePos());

    QGraphicsScene::mousePressEvent(event);
}

void SceneView::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "SceneView::mouseMoveEvent\n");

    if (preview != nullptr && (m_isDrawRect || m_isRectSelection))
    {
        constexpr int gap = 30;
        qreal snappedX = std::round(event->scenePos().x() / gap) * gap;
        qreal snappedY = std::round(event->scenePos().y() / gap) * gap;
        
        QRectF rect(startPoint, QPointF(snappedX, snappedY));
        rect = rect.normalized();
        preview->setRect(rect);
    }
    else if (m_isLassoSelection && lassoPreview != nullptr)
    {
        m_lassoPath.lineTo(event->scenePos());
        lassoPreview->setPath(m_lassoPath);
    }

    emit sceneMouseMove(event->scenePos());
    QGraphicsScene::mouseMoveEvent(event);
}

void SceneView::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "SceneView::mouseReleaseEvent\n");

    if (preview != nullptr)
    {
        if (m_isRectSelection)
        {
            QRectF selectionRect = preview->rect().normalized();
            QPainterPath path;
            path.addRect(selectionRect);
            setSelectionArea(path);
            m_isRectSelection = false;
        }
        
        removeItem(preview);
        delete preview;
        preview = nullptr;
        m_isDrawRect = false;
    }
    else if (lassoPreview != nullptr)
    {
        if (m_isLassoSelection)
        {
            m_lassoPath.closeSubpath();
            setSelectionArea(m_lassoPath);
            m_isLassoSelection = false;
        }
        
        removeItem(lassoPreview);
        delete lassoPreview;
        lassoPreview = nullptr;
    }

    emit sceneMouseRelease(event->scenePos());
    QGraphicsScene::mouseReleaseEvent(event);
}

void SceneView::handleDrawRectPreview() { m_isDrawRect = true; }
