#include "scene.hpp"

Scene::Scene(qreal g) : m_gap(g) { }

void Scene::add(Layer* layer)
{
    addItem(layer);
}

void Scene::remove(Layer* layer) { removeItem(layer); }

void Scene::drawBackground(QPainter* painter, const QRectF& rect)
{
    painter->fillRect(rect, QColor("#141414"));

    if (!drawGrid)
        return;

    painter->setPen(QPen(QColor("#1c1c1c")));

    for (auto left = std::floor(rect.left() / m_gap) * m_gap; left < rect.right(); left += m_gap)
    {
        painter->drawLine(left, rect.top(), left, rect.bottom());
    }

    for (auto top = std::floor(rect.top() / m_gap) * m_gap; top < rect.bottom(); top += m_gap)
    {
        painter->drawLine(rect.left(), top, rect.right(), top);
    }
}

void Scene::nSubstrateCreate()
{
    m_context = std::make_optional(CurrentLayerContext{ Layer::Type::NSubstrate, { 0.0f, 0.0f }, { 0.0f, 0.0f } });
    update();
}

void Scene::pSubstrateCreate()
{
    m_context = std::make_optional(CurrentLayerContext{ Layer::Type::PSubstrate, { 0.0f, 0.0f }, { 0.0f, 0.0f } });
    update();
}

void Scene::nSourceCreate()
{
    m_context = std::make_optional(CurrentLayerContext{ Layer::Type::NSource, { 0.0f, 0.0f }, { 0.0f, 0.0f } });
    update();
}

void Scene::pSourceCreate()
{
    m_context = std::make_optional(CurrentLayerContext{ Layer::Type::PSource, { 0.0f, 0.0f }, { 0.0f, 0.0f } });
    update();
}

void Scene::nDrainCreate()
{
    m_context = std::make_optional(CurrentLayerContext{ Layer::Type::NDrain, { 0.0f, 0.0f }, { 0.0f, 0.0f } });
    update();
}

void Scene::pDrainCreate()
{
    m_context = std::make_optional(CurrentLayerContext{ Layer::Type::PDrain, { 0.0f, 0.0f }, { 0.0f, 0.0f } });
    update();
}

void Scene::oxideCreate()
{
    m_context = std::make_optional(CurrentLayerContext{ Layer::Type::Oxide, { 0.0f, 0.0f }, { 0.0f, 0.0f } });
    update();
}

void Scene::polysiliconCreate()
{
    m_context = std::make_optional(CurrentLayerContext{ Layer::Type::Polysilicon, { 0.0f, 0.0f }, { 0.0f, 0.0f } });
    update();
}

void Scene::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_G)
    {
        drawGrid = !drawGrid;
    }
    if (event->key() == Qt::Key_Delete && m_activeLayer)
    {
        remove(m_activeLayer);
        m_activeLayer = nullptr;
    }
    update();
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{   
    if (m_context.has_value())
    {
        m_context->leftTopCorner = event->scenePos();
        m_preview = addRect(QRectF(m_context->leftTopCorner, m_context->leftTopCorner),
                            QPen(Qt::darkGray, 2),
                            QBrush(Qt::transparent));
        m_isAdd = true;

        event->accept();
        return;
    }

    if (QGraphicsItem* item = itemAt(event->scenePos(), QTransform()); item != nullptr)
    {
        m_activeLayer = dynamic_cast<Layer*>(item);
        m_activeLayer->setBorderColor(QColor("#0ac423"));
        m_activeLayer->setBorderWidth(2);
    }

    QGraphicsScene::mousePressEvent(event);
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_context.has_value() & m_isAdd)
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

void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_context.has_value() && m_isAdd)
    {
        QRectF rect(m_context->leftTopCorner, event->scenePos());
        rect = rect.normalized();

        Layer* newLayer = new Layer(rect, m_context->type);
        add(newLayer);

        m_context = std::nullopt;

        if (m_preview)
        {
            removeItem(m_preview);
            delete m_preview;
            m_preview = nullptr;
        }

        m_isAdd = false;
        this->views().first()->setCursor(Qt::ArrowCursor);
    }

    QGraphicsScene::mouseReleaseEvent(event);
}










