#include "scene.hpp"

Scene::Scene(qreal g) : m_gap(g) { }

void Scene::add(Layer* layer) { addItem(layer); }

void Scene::remove(Layer* layer) { removeItem(layer); }

void Scene::drawBackground(QPainter* painter, const QRectF& rect)
{
    if (!drawGrid)
        return;

    painter->setPen(QPen(Qt::red));

    for (auto left = std::floor(rect.left() / m_gap) * m_gap; left < rect.right(); left += m_gap)
    {
        painter->drawLine(left, rect.top(), left, rect.bottom());
    }

    for (auto top = std::floor(rect.top() / m_gap) * m_gap; top < rect.bottom(); top += m_gap)
    {
        painter->drawLine(rect.left(), top, rect.right(), top);
    }
}

void Scene::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_G)
        drawGrid = !drawGrid;
    update();
}

void Scene::nSubstrate()
{
    currentLayer = std::make_optional(Layer::Type::NSubstrate);
    update();
}

void Scene::pSubstrate()
{
    currentLayer = std::make_optional(Layer::Type::PSubstrate);
    update();
}

void Scene::nSource()
{
    currentLayer = std::make_optional(Layer::Type::NSource);
    update();
}

void Scene::pSource()
{
    currentLayer = std::make_optional(Layer::Type::PSource);
    update();
}

void Scene::nDrain()
{
    currentLayer = std::make_optional(Layer::Type::NDrain);
    update();
}

void Scene::pDrain()
{
    currentLayer = std::make_optional(Layer::Type::PDrain);
    update();
}

void Scene::oxide()
{
    currentLayer = std::make_optional(Layer::Type::Oxide);
    update();
}

void Scene::polysilicon()
{
    currentLayer = std::make_optional(Layer::Type::Polysilicon);
    update();
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (currentLayer.has_value())
    {
        qDebug() << "CurrentLayer is active\n";
    }

    QGraphicsScene::mousePressEvent(event);
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (currentLayer.has_value())
    {
        this->views().first()->setCursor(Qt::CrossCursor);
    }

    QGraphicsScene::mouseMoveEvent(event);
}









