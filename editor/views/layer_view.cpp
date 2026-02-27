#include "layer_view.hpp"

LayerView::LayerView(const QRectF& r, Style s) : id(IdGenerator::generate()), m_rect(r), m_style(s)
{
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsFocusable);
}

QRectF LayerView::boundingRect() const
{
    return m_rect;
}

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

    m_prevPos = scenePos();

    setFocus();

    if (!m_resizeDirection)
    {
        m_isDrag = true;
        m_start = event->pos();
    }

    event->accept();
    emit press(id);
}

void LayerView::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "LayerView::mouseMoveEvent\n");

    if (m_isDrag)
    {
        setPos(mapToScene(event->pos() - m_start));
    }
    else
    {
        prepareGeometryChange();

        switch (m_resizeDirection)
        {
            case ResizeDirection::Left:
            {
                m_rect.setLeft(event->pos().x());
                update();
                break;
            }
            case ResizeDirection::Top:
            {
                m_rect.setTop(event->pos().y());
                update();
                break;
            }
            case ResizeDirection::Right:
            {
                m_rect.setRight(event->pos().x());
                update();
                break;
            }
            case ResizeDirection::Bottom:
            {
                m_rect.setBottom(event->pos().y());
                update();
                break;
            }
            case (ResizeDirection::Left | ResizeDirection::Top):
            {
                m_rect.setTopLeft(event->pos());
                update();
                break;
            }
            case (ResizeDirection::Top | ResizeDirection::Right):
            {
                m_rect.setTopRight(event->pos());
                update();
                break;
            }
            case (ResizeDirection::Right | ResizeDirection::Bottom):
            {
                m_rect.setBottomLeft(event->pos());
                update();
                break;
            }
            case (ResizeDirection::Bottom | ResizeDirection::Left):
            {
                m_rect.setBottomLeft(event->pos());
                update();
                break;
            }
        }
    }
    event->accept();
}

void LayerView::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "LayerView::mouseReleaseEvent\n");

    if (m_isDrag)
        emit moved(id, m_prevPos, scenePos());

    m_isDrag = false;
    event->accept();
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
    fprintf(stderr, "LayerView::hoverMoveEvent\n");
    m_resizeDirection = ResizeDirection::None;

    auto inDirection = [](qreal coord, qreal sentinel, qreal tolerance)
    {
        return std::abs(sentinel - coord) <= tolerance;
    };

    auto [x1, y1] = m_rect.topLeft();
    auto [x2, y2] = m_rect.bottomRight();
    auto curr = event->pos();
    constexpr auto tolerance = 7;

    if (inDirection(curr.x(), x1, tolerance))
        m_resizeDirection |= 1 << 3;

    if (inDirection(curr.y(), y1, tolerance))
        m_resizeDirection |= 1 << 2;

    if (inDirection(curr.x(), x2, tolerance))
        m_resizeDirection |= 1 << 1;

    if (inDirection(curr.y(), y2, tolerance))
        m_resizeDirection |= 1 << 0;

    switch (m_resizeDirection)
    {
        case ResizeDirection::Left:
        {
            setCursor(Qt::SizeHorCursor);
            break;
        }
        case ResizeDirection::Top:
        {
            setCursor(Qt::SizeVerCursor);
            break;
        }
        case ResizeDirection::Right:
        {
            setCursor(Qt::SizeHorCursor);
            break;
        }
        case ResizeDirection::Bottom:
        {
            setCursor(Qt::SizeVerCursor);
            break;
        }
        case (ResizeDirection::Left | ResizeDirection::Top):
        {
            setCursor(Qt::SizeBDiagCursor);
            break;
        }
        case (ResizeDirection::Top | ResizeDirection::Right):
        {
            setCursor(Qt::SizeFDiagCursor);
            break;
        }
        case (ResizeDirection::Right | ResizeDirection::Bottom):
        {
            setCursor(Qt::SizeBDiagCursor);
            break;
        }
        case (ResizeDirection::Bottom | ResizeDirection::Left):
        {
            setCursor(Qt::SizeFDiagCursor);
            break;
        }
        default:
        {
            setCursor(Qt::ArrowCursor);
        }
    }
    QGraphicsItem::hoverMoveEvent(event);
}

void LayerView::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    fprintf(stderr, "LayerView::hoverEnterEvent");
    m_style.background = m_baseColor;
    update();
    QGraphicsItem::hoverLeaveEvent(event);
}







