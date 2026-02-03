#include "layer_view.hpp"

LayerView::LayerView(const QRectF& r, Type type) : m_rect(r), m_type(type)
{
    m_color = colorMap[type];
    this->setFlag(QGraphicsItem::ItemIsSelectable);
    this->setFlag(QGraphicsItem::ItemIsMovable);
}

QRectF const LayerView::getRect() const { return m_rect; }
QRectF LayerView::getRect() { return m_rect; }

void LayerView::setBorderColor(QColor color)
{
    m_border.color = color;
    update();
}

void LayerView::setBorderWidth(qreal width)
{
    m_border.width = width;
    update();
}

QRectF LayerView::boundingRect() const
{
    return QRectF(m_rect.x(), m_rect.y(), m_rect.width(), m_rect.height());
}

void LayerView::paint(QPainter *painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPen pen(m_border.color, m_border.width);
    painter->setPen(pen);

    painter->setBrush(m_color);
    painter->drawRect(boundingRect());
}

QLabel* LayerView::info()
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
