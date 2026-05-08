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

    constexpr int gap = 30;
    qreal snappedX = std::round(event->scenePos().x() / gap) * gap;
    qreal snappedY = std::round(event->scenePos().y() / gap) * gap;
    QPointF snappedPos(snappedX, snappedY);

    if (m_isDrawRect || m_isRectSelection)
    {
        startPoint = snappedPos;
        preview = addRect(QRectF(startPoint, startPoint), QPen(QColor(Qt::gray), 1, Qt::DashLine));
    }
    else if (m_isDrawPolygon)
    {
        if (m_polygonPoints.isEmpty())
        {
            m_polygonPoints << snappedPos << snappedPos;
            QPainterPath path;
            path.moveTo(snappedPos);
            path.lineTo(snappedPos);
            
            QPen pen(Qt::magenta, 30, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
            polygonPreview = addPath(path, pen);
        }
        else
        {
            QPointF lastFixed = m_polygonPoints.at(m_polygonPoints.size() - 2);
            QPointF current = snappedPos;

            // Manhattan constraint
            if (std::abs(current.x() - lastFixed.x()) > std::abs(current.y() - lastFixed.y()))
                current.setY(lastFixed.y());
            else
                current.setX(lastFixed.x());
            
            snappedPos = current;
            m_polygonPoints.back() = snappedPos;
            m_polygonPoints << snappedPos; // Add new moving point
            
            QPainterPath path;
            path.moveTo(m_polygonPoints.first());
            for(int i = 1; i < m_polygonPoints.size(); ++i)
                path.lineTo(m_polygonPoints.at(i));
            polygonPreview->setPath(path);
        }
    }
    else if (m_isLassoSelection)
    {
        m_lassoPath = QPainterPath();
        m_lassoPath.moveTo(event->scenePos());
        lassoPreview = addPath(m_lassoPath, QPen(QColor(Qt::blue), 1, Qt::DashLine));
    }

    emit sceneClick(snappedPos);

    QGraphicsScene::mousePressEvent(event);
}

void SceneView::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "SceneView::mouseMoveEvent\n");

    constexpr int gap = 30;
    qreal snappedX = std::round(event->scenePos().x() / gap) * gap;
    qreal snappedY = std::round(event->scenePos().y() / gap) * gap;
    QPointF snappedPos(snappedX, snappedY);

    if (preview != nullptr && (m_isDrawRect || m_isRectSelection))
    {
        QRectF rect(startPoint, snappedPos);
        rect = rect.normalized();
        preview->setRect(rect);
    }
    else if (m_isDrawPolygon)
    {
        if (polygonPreview == nullptr)
        {
            if (m_cursorPreview == nullptr)
            {
                m_cursorPreview = addRect(QRectF(-15, -15, 30, 30), QPen(Qt::NoPen), QBrush(QColor(255, 0, 255, 127)));
                m_cursorPreview->setZValue(1000);
            }
            m_cursorPreview->setPos(snappedPos);
        }
        else if (m_cursorPreview != nullptr)
        {
            removeItem(m_cursorPreview);
            delete m_cursorPreview;
            m_cursorPreview = nullptr;
        }

        if (polygonPreview != nullptr && m_polygonPoints.size() >= 2)
        {
            QPointF lastFixed = m_polygonPoints.at(m_polygonPoints.size() - 2);
            QPointF current = snappedPos;

            // Manhattan constraint
            if (std::abs(current.x() - lastFixed.x()) > std::abs(current.y() - lastFixed.y()))
                current.setY(lastFixed.y());
            else
                current.setX(lastFixed.x());
            
            m_polygonPoints.back() = current;
            
            QPainterPath path;
            path.moveTo(m_polygonPoints.first());
            for(int i = 1; i < m_polygonPoints.size(); ++i)
                path.lineTo(m_polygonPoints.at(i));
            polygonPreview->setPath(path);
            
            emit sceneMouseMove(current);
            return;
        }
    }
    else if (m_isLassoSelection && lassoPreview != nullptr)
    {
        m_lassoPath.lineTo(event->scenePos());
        lassoPreview->setPath(m_lassoPath);
    }

    emit sceneMouseMove(snappedPos);
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

void SceneView::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "SceneView::mouseDoubleClickEvent\n");

    constexpr int gap = 30;
    qreal snappedX = std::round(event->scenePos().x() / gap) * gap;
    qreal snappedY = std::round(event->scenePos().y() / gap) * gap;
    QPointF snappedPos(snappedX, snappedY);

    emit sceneMouseDoubleClick(snappedPos);

    if (m_isDrawPolygon && polygonPreview != nullptr)
    {
        removeItem(polygonPreview);
        delete polygonPreview;
        polygonPreview = nullptr;
        m_isDrawPolygon = false;
        m_polygonPoints.clear();
    }

    if (m_cursorPreview != nullptr)
    {
        removeItem(m_cursorPreview);
        delete m_cursorPreview;
        m_cursorPreview = nullptr;
    }

    QGraphicsScene::mouseDoubleClickEvent(event);
}

void SceneView::handleDrawRectPreview() 
{ 
    m_isDrawRect = true; 
    m_isDrawPolygon = false;
    m_isRectSelection = false;
    m_isLassoSelection = false;
    
    if (m_cursorPreview != nullptr)
    {
        removeItem(m_cursorPreview);
        delete m_cursorPreview;
        m_cursorPreview = nullptr;
    }
}
void SceneView::handleDrawPolygonPreview() 
{ 
    m_isDrawPolygon = true; 
    m_isDrawRect = false;
    m_isRectSelection = false;
    m_isLassoSelection = false;

    if (m_cursorPreview != nullptr)
    {
        removeItem(m_cursorPreview);
        delete m_cursorPreview;
        m_cursorPreview = nullptr;
    }
}
