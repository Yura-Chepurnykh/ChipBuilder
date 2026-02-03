#include "scene_view.hpp"

SceneView::SceneView(qreal g) : m_gap(g)
{

}

void SceneView::drawBackground(QPainter* painter, const QRectF& rect)
{
    painter->fillRect(rect, QColor("#141414"));

    if (!m_drawGrid)
        return;

    painter->setPen(QPen((QColor("#1c1c1c")), 2));

    for (auto left = std::floor(rect.left() / m_gap) * m_gap; left < rect.right(); left += m_gap)
    {
        painter->drawLine(left, rect.top(), left, rect.bottom());
    }

    for (auto top = std::floor(rect.top() / m_gap) * m_gap; top < rect.bottom(); top += m_gap)
    {
        painter->drawLine(rect.left(), top, rect.right(), top);
    }
}

void SceneView::keyPressEvent(QKeyEvent* event)
{
    switch(event->key())
    {
    case Qt::Key_G:
    {
        m_drawGrid = !m_drawGrid;
        m_metal = !m_metal;

        qInfo() << "Key M\n";

        break;
    }
    case Qt::Key_Delete:
    {
        if (m_activeLayer)
        {
            remove(m_activeLayer);
            m_activeLayer = nullptr;
        } break;
    }
    case Qt::Key_M:
    {
        m_metal = !m_metal;
        qInfo() << "Key M\n";
        break;
    }

    }
    update();
}

void SceneView::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_context.has_value())
    {
        m_context->leftTopCorner = event->scenePos();
        m_preview = addRect(QRectF(m_context->leftTopCorner, m_context->leftTopCorner),
                            QPen(Qt::darkGray, 2),
                            QBrush(Qt::transparent));
        m_layer = true;

        event->accept();
        return;
    }

    if (QGraphicsItem* item = itemAt(event->scenePos(), QTransform()); item != nullptr)
    {
        m_activeLayer = dynamic_cast<LayerView*>(item);
        if (m_activeLayer)
        {
            m_activeLayer->setBorderColor(QColor("#0ac423"));
            m_activeLayer->setBorderWidth(2);
        }
    }
    // else
    // {
    //     m_activeLayer->setBorderWidth(-1);
    // }
    qInfo() << "before m_metal\n";

    if (m_metal)
    {
        qInfo() << "m_metal\n";
    }

    QGraphicsScene::mousePressEvent(event);
}

void SceneView::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_metal)
    {
        m_metal = false;
    }
}

void SceneView::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_context.has_value() & m_layer)
    {
        QRectF rect(m_context->leftTopCorner, event->scenePos());
        rect = rect.normalized();

        if (m_preview)
            m_preview->setRect(rect);

        this->views().first()->setCursor(Qt::CrossCursor);

        event->accept();
        return;
    }

    QGraphicsScene::mouseMoveEvent(event);
}

void SceneView::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_context.has_value() && m_layer)
    {
        QRectF rect(m_context->leftTopCorner, event->scenePos());
        rect = rect.normalized();

        LayerView* newLayer = new LayerView(rect, m_context->type);
        add(newLayer);

        m_context = std::nullopt;

        if (m_preview)
        {
            removeItem(m_preview);
            delete m_preview;
            m_preview = nullptr;
        }

        m_layer = false;
        this->views().first()->setCursor(Qt::ArrowCursor);
    }

    QGraphicsScene::mouseReleaseEvent(event);
}

