#include "layer.hpp"

Layer::Layer(const QRectF& r, Type type) : m_rect(r), m_type(type)
{
    m_color = colorMap[type];
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

QLabel* Layer::info()
{
    switch(m_type)
    {
        case Type::NSubstrate:  return new QLabel("N-Substrate");
        case Type::PSubstrate:  return new QLabel("P-Substrate");
        case Type::NSource:     return new QLabel("N-Source");
        case Type::PSource:     return new QLabel("P-Source");
        case Type::NDrain:      return new QLabel("N-Drain");
        case Type::PDrain:      return new QLabel("P-Drain");
        case Type::Oxide:       return new QLabel("Oxide");
        case Type::Polysilicon: return new QLabel("Polysilicon");
        default: return new QLabel("Unknown layer");
    }
}
