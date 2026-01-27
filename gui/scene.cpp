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













