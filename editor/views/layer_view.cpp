#include "layer_view.hpp"
#include <QGraphicsScene>
#include <QMenu>
#include <QInputDialog>
#include <QGraphicsSceneContextMenuEvent>

LayerView::LayerView(const QRectF& r, Style s, const QString& name) : 
    id(IdGenerator::generate()), m_rect(r), m_style(s), m_name(name)
{
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsFocusable);
}

LayerView::~LayerView()
{
    if (m_label) {
        delete m_label;
        m_label = nullptr;
    }
}

QRectF LayerView::boundingRect() const
{
    return m_rect.adjusted(-50, -50, 50, 50);
}

void LayerView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->fillRect(m_rect, QColor(m_style.background));
    
    if (m_isDRCError)
    {
        painter->fillRect(m_rect, QColor(255, 0, 0, 255)); // Solid red on top
    }

    QPen pen = m_style.pen;

    if (isSelected())
    {
        auto drawDim = [&](const QPointF& p1, const QPointF& p2, const QPointF& normal, bool isHor) {
            qreal len = isHor ? std::abs(p2.x() - p1.x()) : std::abs(p2.y() - p1.y());
            if (len < 1) return;
            
            int lambda = std::round(len / 30.0);
            QString text = QString::number(lambda) + " λ";
            
            QPointF offset = normal * 15;
            QPointF s = p1 + offset;
            QPointF e = p2 + offset;
            
            painter->setPen(QPen(Qt::white, 1));
            painter->drawLine(s, e);
            
            // Ticks
            QPointF t = isHor ? QPointF(0, 5) : QPointF(5, 0);
            painter->drawLine(s - t, s + t);
            painter->drawLine(e - t, e + t);
            
            // Text
            painter->setFont(QFont("Arial", 10, QFont::Bold));
            QRectF br = painter->fontMetrics().boundingRect(text);
            QPointF center = (s + e) / 2.0;
            br.moveCenter(center);
            painter->fillRect(br.adjusted(-2, 0, 2, 0), QColor(43, 43, 43, 200));
            painter->drawText(br, Qt::AlignCenter, text);
        };

        // Top
        drawDim(m_rect.topLeft(), m_rect.topRight(), QPointF(0, -1), true);
        // Bottom
        drawDim(m_rect.bottomLeft(), m_rect.bottomRight(), QPointF(0, 1), true);
        // Left
        drawDim(m_rect.topLeft(), m_rect.bottomLeft(), QPointF(-1, 0), false);
        // Right
        drawDim(m_rect.topRight(), m_rect.bottomRight(), QPointF(1, 0), false);
    }
}

void LayerView::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *raiseAction = menu.addAction("Raise Layer (zLevel++)");
    QAction *lowerAction = menu.addAction("Lower Layer (zLevel--)");
    QAction *setAction = menu.addAction("Set Level...");
    
    QAction *selectedAction = menu.exec(event->screenPos());
    
    if (selectedAction == raiseAction)
    {
        emit raiseRequested(id);
    }
    else if (selectedAction == lowerAction)
    {
        emit lowerRequested(id);
    }
    else if (selectedAction == setAction)
    {
        bool ok;
        int newLevel = QInputDialog::getInt(nullptr, "Set Layer Level",
                                            "zLevel:", zLevel, -100, 100, 1, &ok);
        if (ok)
        {
            emit setLevelRequested(id, newLevel);
        }
    }
    event->accept();
}

QVariant LayerView::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemZValueHasChanged)
    {
        update();
    }
    if (change == ItemSelectedHasChanged)
    {
        update();
    }

    if (change == ItemSceneHasChanged || change == ItemVisibleHasChanged)
    {
        if (m_label) {
            m_label->hide();
        }
    }

    return QGraphicsItem::itemChange(change, value);
}

void LayerView::setRect(qreal x, qreal y, qreal w, qreal h)
{
    prepareGeometryChange();
    m_rect = QRectF(x, y, w, h);
    update();
}

void LayerView::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "LayerView::mousePressEvent\n");

    m_prevPos = scenePos();
    m_prevRect = QRectF(scenePos().x(), scenePos().y(), m_rect.width(), m_rect.height());

    setFocus();

    if (!m_resizeDirection)
    {
        m_isDrag = true;
        m_start = event->pos();
    }

    QGraphicsItem::mousePressEvent(event); // Call base class for selection
    event->accept();
    emit press(id);
}

void LayerView::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "LayerView::mouseMoveEvent\n");

    if (m_isDrag)
    {
        QPointF newPos = mapToScene(event->pos() - m_start);
        constexpr int gap = 30;
        qreal snappedX = std::round(newPos.x() / gap) * gap;
        qreal snappedY = std::round(newPos.y() / gap) * gap;
        QPointF snappedPos(snappedX, snappedY);
        
        QPointF delta = snappedPos - scenePos();
        
        if (isSelected())
        {
            for (auto* item : scene()->selectedItems())
            {
                if (auto* layer = dynamic_cast<LayerView*>(item))
                {
                    layer->setPos(layer->scenePos() + delta);
                    emit layer->geometryChanged(layer->id, QRectF(layer->scenePos().x(), layer->scenePos().y(), layer->m_rect.width(), layer->m_rect.height()));
                }
            }
        }
        else
        {
            setPos(snappedPos);
            emit geometryChanged(id, QRectF(scenePos().x(), scenePos().y(), m_rect.width(), m_rect.height()));
        }
    }
    else
    {
        prepareGeometryChange();
        constexpr int gap = 30;

        auto snap = [gap](qreal val) {
            return std::round(val / gap) * gap;
        };

        QPointF posInScene = mapToScene(event->pos());
        qreal snappedX = snap(posInScene.x());
        qreal snappedY = snap(posInScene.y());
        QPointF localSnapped = mapFromScene(QPointF(snappedX, snappedY));

        switch (m_resizeDirection)
        {
            case ResizeDirection::Left:
            {
                qreal diff = localSnapped.x() - m_rect.left();
                m_rect.setLeft(localSnapped.x());
                setPos(mapToScene(QPointF(diff, 0)));
                break;
            }
            case ResizeDirection::Top:
            {
                qreal diff = localSnapped.y() - m_rect.top();
                m_rect.setTop(localSnapped.y());
                setPos(mapToScene(QPointF(0, diff)));
                break;
            }
            case ResizeDirection::Right:
            {
                m_rect.setRight(localSnapped.x());
                break;
            }
            case ResizeDirection::Bottom:
            {
                m_rect.setBottom(localSnapped.y());
                break;
            }
            case (ResizeDirection::Left | ResizeDirection::Top):
            {
                qreal diffX = localSnapped.x() - m_rect.left();
                qreal diffY = localSnapped.y() - m_rect.top();
                m_rect.setTopLeft(localSnapped);
                setPos(mapToScene(QPointF(diffX, diffY)));
                break;
            }
            case (ResizeDirection::Top | ResizeDirection::Right):
            {
                qreal diffY = localSnapped.y() - m_rect.top();
                m_rect.setTopRight(localSnapped);
                setPos(mapToScene(QPointF(0, diffY)));
                break;
            }
            case (ResizeDirection::Right | ResizeDirection::Bottom):
            {
                m_rect.setBottomRight(localSnapped);
                break;
            }
            case (ResizeDirection::Bottom | ResizeDirection::Left):
            {
                qreal diffX = localSnapped.x() - m_rect.left();
                m_rect.setBottomLeft(localSnapped);
                setPos(mapToScene(QPointF(diffX, 0)));
                break;
            }
        }
        emit geometryChanged(id, QRectF(scenePos().x(), scenePos().y(), m_rect.width(), m_rect.height()));
        update();
    }
    event->accept();
}

void LayerView::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    fprintf(stderr, "LayerView::mouseReleaseEvent\n");

    if (m_isDrag)
    {
        if (isSelected())
        {
            for (auto* item : scene()->selectedItems())
            {
                if (auto* layer = dynamic_cast<LayerView*>(item))
                {
                    // Ensure snapped
                    constexpr int gap = 30;
                    qreal snappedX = std::round(layer->scenePos().x() / gap) * gap;
                    qreal snappedY = std::round(layer->scenePos().y() / gap) * gap;
                    layer->setPos(snappedX, snappedY);

                    if (layer->m_prevPos != layer->scenePos())
                        emit layer->moved(layer->id, layer->m_prevPos, layer->scenePos());
                }
            }
        }
        else
        {
            constexpr int gap = 30;
            qreal snappedX = std::round(scenePos().x() / gap) * gap;
            qreal snappedY = std::round(scenePos().y() / gap) * gap;
            setPos(snappedX, snappedY);
            emit moved(id, m_prevPos, scenePos());
        }
    }
    else if (m_resizeDirection != ResizeDirection::None)
    {
        QRectF newRectInScene(scenePos().x(), scenePos().y(), m_rect.width(), m_rect.height());
        emit resized(id, m_prevRect, newRectInScene);
    }

    m_isDrag = false;
    event->accept();
}

void LayerView::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    fprintf(stderr, "LayerView::hoverEnterEvent");
    m_baseColor = m_style.background;
    m_style.background = m_style.background.lighter(130);
    
    if (!m_label && !m_name.isEmpty()) {
        m_label = new QLabel();
        m_label->setText(m_name);
        m_label->setStyleSheet("background-color: #2b2b2b; color: #ffffff; border: 1px solid #444444; padding: 2px; border-radius: 3px;");
        m_label->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_label->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    }
    
    if (m_label) {
        m_label->show();
        m_label->move(event->screenPos() + QPoint(10, 10));
    }

    update();
    QGraphicsItem::hoverEnterEvent(event);
}

void LayerView::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    fprintf(stderr, "LayerView::hoverMoveEvent\n");
    m_resizeDirection = ResizeDirection::None;

    if (m_label) {
        m_label->move(event->screenPos() + QPoint(10, 10));
    }

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
    fprintf(stderr, "LayerView::hoverLeaveEvent\n");
    m_style.background = m_baseColor;
    
    if (m_label) {
        m_label->hide();
    }
    
    update();
    QGraphicsItem::hoverLeaveEvent(event);
}
