#include "metal_view.hpp"
#include "id_generator.hpp"

MetalView::MetalView(QSharedPolygon p, Style s) : id(IdGenerator::generate()), m_path(p), m_style(s)
{
    m_style.pen.setColor(Qt::magenta);
    m_style.pen.setWidth(50);
    m_style.pen.setStyle(Qt::SolidLine);
    m_style.pen.setJoinStyle(Qt::MiterJoin);
    m_style.pen.setCapStyle(Qt::FlatCap);

    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
}

QRectF MetalView::boundingRect() const
{
    return shape().boundingRect().adjusted(-50, -50, 50, 50);
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
    if (isSelected())
    {
        pen.setColor(QColor(173, 216, 230)); // Light Blue
    }
    painter->setPen(pen);

    for (int i = 1; i < m_path.size(); ++i)
        painter->drawLine(*m_path[i-1], *m_path[i]);

    if (isSelected())
    {
        for (int i = 1; i < m_path.size(); ++i)
        {
            QPointF p1 = *m_path[i-1];
            QPointF p2 = *m_path[i];
            
            qreal len = std::sqrt(std::pow(p2.x() - p1.x(), 2) + std::pow(p2.y() - p1.y(), 2));
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
