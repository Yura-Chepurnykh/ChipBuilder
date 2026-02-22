#include "layer_view.hpp"

LayerView::LayerView(const QRectF& r, Style s) : m_rect(r), m_style(s), id(IdGenerator::generate())
{
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsFocusable);
}

QRectF LayerView::boundingRect() const { return m_rect; }

void LayerView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(QColor(m_style.background));
    painter->setPen(m_style.pen);
    painter->drawRect(m_rect);
}

void LayerView::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "LayerView::mousePressEvent\n");

    setFocus();

    m_isDrag = true;
    m_start = event->pos();
    event->accept();
    emit press(id);
    //QGraphicsItem::mousePressEvent(event);
}

void LayerView::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "LayerView::mouseMoveEvent\n");
    if (m_isDrag)
    {
        setPos(mapToScene(event->pos() - m_start));
    }
    event->accept();
    //QGraphicsItem::mouseMoveEvent(event);
}

void LayerView::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "LayerView::mouseReleaseEvent\n");
    m_isDrag = false;
    event->accept();
    //QGraphicsItem::mouseReleaseEvent(event);
}

void LayerView::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    fprintf(stderr, "LayerView::hoverEnterEvent");
    m_baseColor = m_style.background;
    m_style.background = m_style.background.lighter(130);
    update();
    QGraphicsItem::hoverEnterEvent(event);
}

void LayerView::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    fprintf(stderr, "LayerView::hoverMoveEvent");
    QGraphicsItem::hoverMoveEvent(event);
}

void LayerView::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    fprintf(stderr, "LayerView::hoverEnterEvent");
    m_style.background = m_baseColor;
    update();
    QGraphicsItem::hoverLeaveEvent(event);
}







