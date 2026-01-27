#include "layer.hpp"

Layer::Layer(const QRectF& r, const QColor& color) : m_rect(r), m_color(color)
{
    this->setFlag(QGraphicsItem::ItemIsSelectable);
    this->setFlag(QGraphicsItem::ItemIsMovable);
}

QRectF const Layer::getRect() const { return m_rect; }
QRectF Layer::getRect() { return m_rect; }

QRectF Layer::boundingRect() const
{
    return QRectF(m_rect.x(), m_rect.y(), m_rect.width(), m_rect.height());
}

void Layer::paint(QPainter *painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setBrush(m_color);
    painter->drawRect(boundingRect());
}

