#include "metal_view.hpp"
#include "id_generator.hpp"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QCursor>
#include <QVector2D>
#include <QGraphicsScene>

#include <QMenu>
#include <QInputDialog>
#include <QGraphicsSceneContextMenuEvent>

MetalView::MetalView(QSharedPolygon p, Style s, int thickness) : 
    id(IdGenerator::generate()), m_style(s), m_path(p), m_thickness(thickness)
{
    m_style.pen.setColor(Qt::magenta);
    m_style.pen.setWidth(m_thickness * 30); // lambda to pixels
    m_style.pen.setStyle(Qt::SolidLine);
    m_style.pen.setJoinStyle(Qt::MiterJoin);
    m_style.pen.setCapStyle(Qt::FlatCap);

    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptHoverEvents(true);
}

void MetalView::setThickness(int thickness)
{
    prepareGeometryChange();
    m_thickness = thickness;
    m_style.pen.setWidth(m_thickness * 30);
    update();
}

void MetalView::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *setThicknessAction = menu.addAction("Set Thickness...");
    
    QAction *selectedAction = menu.exec(event->screenPos());
    
    if (selectedAction == setThicknessAction)
    {
        bool ok;
        int newThickness = QInputDialog::getInt(nullptr, "Set Metal Thickness",
                                            "Thickness (lambda):", m_thickness, 1, 20, 1, &ok);
        if (ok)
        {
            emit thicknessChanged(id, newThickness);
        }
    }
    event->accept();
}

void MetalView::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_draggedSegmentIdx != -1 || m_draggedPointIdx != -1)
    {
        m_isResizing = true;
        m_pressPos = event->pos();
        m_initialPoints.clear();
        for (const auto& p : m_path)
            m_initialPoints.push_back(*p);
        event->accept();
    }
    else
    {
        QGraphicsItem::mousePressEvent(event);
    }
}

void MetalView::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_isResizing)
    {
        QPointF delta = event->pos() - m_pressPos;
        
        constexpr int gap = 30;
        auto snap = [gap](qreal val) {
            return std::round(val / gap) * gap;
        };

        if (m_draggedSegmentIdx != -1)
        {
            prepareGeometryChange();
            QPointF& p1 = *m_path[m_draggedSegmentIdx - 1];
            QPointF& p2 = *m_path[m_draggedSegmentIdx];
            
            const QPointF& initP1 = m_initialPoints[m_draggedSegmentIdx - 1];
            const QPointF& initP2 = m_initialPoints[m_draggedSegmentIdx];

            bool isHor = std::abs(initP1.y() - initP2.y()) < 1.0;
            bool isVer = std::abs(initP1.x() - initP2.x()) < 1.0;

            if (isHor)
            {
                qreal newY = snap(initP1.y() + delta.y());
                p1.setY(newY);
                p2.setY(newY);
            }
            else if (isVer)
            {
                qreal newX = snap(initP1.x() + delta.x());
                p1.setX(newX);
                p2.setX(newX);
            }
            emit geometryChanged(id);
            update();
        }
        else if (m_draggedPointIdx != -1)
        {
            prepareGeometryChange();
            QPointF& p = *m_path[m_draggedPointIdx];
            const QPointF& initP = m_initialPoints[m_draggedPointIdx];
            
            p.setX(snap(initP.x() + delta.x()));
            p.setY(snap(initP.y() + delta.y()));
            emit geometryChanged(id);
            update();
        }
        event->accept();
    }
    else
    {
        QGraphicsItem::mouseMoveEvent(event);
    }
}

void MetalView::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_isResizing)
    {
        m_isResizing = false;
        event->accept();
    }
    else
    {
        QGraphicsItem::mouseReleaseEvent(event);
    }
}

void MetalView::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    QPointF pos = event->pos();
    m_draggedSegmentIdx = -1;
    m_draggedPointIdx = -1;
    setCursor(Qt::ArrowCursor);

    constexpr qreal tolerance = 10.0;
    qreal halfWidth = m_style.pen.width() / 2.0;

    // First check points (ends)
    for (int i = 0; i < m_path.size(); ++i)
    {
        if (QVector2D(pos - *m_path[i]).length() <= tolerance)
        {
            m_draggedPointIdx = i;
            setCursor(Qt::SizeAllCursor);
            return;
        }
    }

    // Then check segments (sides)
    for (int i = 1; i < m_path.size(); ++i)
    {
        QPointF p1 = *m_path[i-1];
        QPointF p2 = *m_path[i];

        bool isHor = std::abs(p1.y() - p2.y()) < 1.0;
        bool isVer = std::abs(p1.x() - p2.x()) < 1.0;

        if (isHor)
        {
            qreal minX = std::min(p1.x(), p2.x());
            qreal maxX = std::max(p1.x(), p2.x());
            if (pos.x() >= minX - tolerance && pos.x() <= maxX + tolerance)
            {
                if (std::abs(pos.y() - (p1.y() - halfWidth)) <= tolerance ||
                    std::abs(pos.y() - (p1.y() + halfWidth)) <= tolerance ||
                    std::abs(pos.y() - p1.y()) <= tolerance) // Also allow clicking middle of segment
                {
                    setCursor(Qt::SizeVerCursor);
                    m_draggedSegmentIdx = i;
                    break;
                }
            }
        }
        else if (isVer)
        {
            qreal minY = std::min(p1.y(), p2.y());
            qreal maxY = std::max(p1.y(), p2.y());
            if (pos.y() >= minY - tolerance && pos.y() <= maxY + tolerance)
            {
                if (std::abs(pos.x() - (p1.x() - halfWidth)) <= tolerance ||
                    std::abs(pos.x() - (p1.x() + halfWidth)) <= tolerance ||
                    std::abs(pos.x() - p1.x()) <= tolerance) // Also allow clicking middle of segment
                {
                    setCursor(Qt::SizeHorCursor);
                    m_draggedSegmentIdx = i;
                    break;
                }
            }
        }
    }
}

void MetalView::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    setCursor(Qt::ArrowCursor);
    QGraphicsItem::hoverLeaveEvent(event);
}

QRectF MetalView::boundingRect() const
{
    return shape().boundingRect().adjusted(-20, -20, 20, 20);
}

QPainterPath MetalView::shape() const
{
    QPainterPath path;

    if (m_path.isEmpty()) return path;

    path.moveTo(*m_path[0]);

    for (int i = 1; i < m_path.size(); ++i)
        path.lineTo(*m_path[i]);

    QPainterPathStroker stroker;
    stroker.setCapStyle(m_style.pen.capStyle());
    stroker.setJoinStyle(m_style.pen.joinStyle());
    stroker.setWidth(m_style.pen.width());

    return stroker.createStroke(path);
}

void MetalView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPen pen = m_style.pen;
    if (m_drcViolated)
    {
        pen.setColor(QColor(255, 0, 0, 180)); // Red
    }
    else if (isSelected())
    {
        pen.setColor(QColor(173, 216, 230)); // Light Blue
    }
    painter->setPen(pen);

    QPainterPath path;
    if (!m_path.isEmpty())
    {
        path.moveTo(*m_path[0]);
        for (int i = 1; i < m_path.size(); ++i)
            path.lineTo(*m_path[i]);
    }
    
    if (m_drcViolated) {
        QPainterPathStroker stroker;
        stroker.setWidth(m_style.pen.width());
        stroker.setCapStyle(m_style.pen.capStyle());
        stroker.setJoinStyle(m_style.pen.joinStyle());
        QPainterPath stroke = stroker.createStroke(path);
        painter->fillPath(stroke, QColor(255, 0, 0, 150)); // More opaque red
    }
    
    painter->drawPath(path);

    if (isSelected())
    {
        double totalLength = 0;
        for (int i = 1; i < m_path.size(); ++i)
        {
            QPointF p1 = *m_path[i-1];
            QPointF p2 = *m_path[i];
            
            qreal len = std::sqrt(std::pow(p2.x() - p1.x(), 2) + std::pow(p2.y() - p1.y(), 2));
            totalLength += len;
            if (len < 5) continue;
            
            int lambda = std::round(len / 30.0);
            QString text = QString::number(lambda) + " λ";
            
            bool isHor = std::abs(p1.y() - p2.y()) < 1.0;
            QPointF normal = isHor ? QPointF(0, -1) : QPointF(-1, 0);
            QPointF offset = normal * (m_style.pen.width() / 2 + 10);
            
            QPointF s = p1 + offset;
            QPointF e = p2 + offset;
            
            painter->setPen(QPen(Qt::white, 1));
            painter->drawLine(s, e);
            
            QPointF t = isHor ? QPointF(0, 5) : QPointF(5, 0);
            painter->drawLine(s - t, s + t);
            painter->drawLine(e - t, e + t);
            
            painter->setFont(QFont("Arial", 10, QFont::Bold));
            QRectF br = painter->fontMetrics().boundingRect(text);
            QPointF center = (s + e) / 2.0;
            br.moveCenter(center);
            painter->fillRect(br.adjusted(-2, 0, 2, 0), QColor(43, 43, 43, 200));
            painter->drawText(br, Qt::AlignCenter, text);
        }

        // Display total area
        if (m_path.size() >= 2)
        {
            // Shoelace area
            double shoelaceArea = 0;
            for (int i = 0; i < m_path.size(); ++i)
            {
                const QPointF& cur = *m_path[i];
                const QPointF& next = *m_path[(i + 1) % m_path.size()];
                shoelaceArea += (cur.x() / 30.0) * (next.y() / 30.0) - (next.x() / 30.0) * (cur.y() / 30.0);
            }
            shoelaceArea = std::abs(shoelaceArea) / 2.0;

            double wireArea = (totalLength / 30.0) * m_thickness;
            double displayArea = (shoelaceArea > 0.1) ? shoelaceArea : wireArea;

            QString areaText = QString("Area: %1 λ²").arg(displayArea, 0, 'f', 1);
            painter->setFont(QFont("Arial", 12, QFont::Bold));
            QRectF areaBr = painter->fontMetrics().boundingRect(areaText);
            
            // Position area text near the first point
            areaBr.moveTopLeft(*m_path[0] + QPointF(20, 20));
            painter->fillRect(areaBr.adjusted(-5, -2, 5, 2), QColor(0, 0, 0, 150));
            painter->setPen(Qt::yellow);
            painter->drawText(areaBr, Qt::AlignCenter, areaText);
        }
    }
}

QVariant MetalView::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged)
    {
        update();
    }
    return QGraphicsItem::itemChange(change, value);
}
