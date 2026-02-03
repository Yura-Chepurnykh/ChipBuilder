#ifndef LAYER_VIEW_H
#define LAYER_VIEW_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QRectF>
#include <QHash>
#include <QColor>
#include <QPainter>
#include <QLabel>

class LayerView : public QGraphicsItem
{
public:
    enum class Type
    {
        NSubstrate,
        PSubstrate,
        NSource,
        PSource,
        NDrain,
        PDrain,
        Oxide,
        Polysilicon
    };

    const QHash<Type, QColor> colorMap
    {
        { Type::NSubstrate,  Qt::gray     },
        { Type::PSubstrate,  Qt::red      },
        { Type::NSource,     Qt::blue     },
        { Type::PSource,     Qt::darkCyan },
        { Type::NDrain,      Qt::cyan     },
        { Type::PDrain,      Qt::yellow   },
        { Type::Oxide,       Qt::green    },
        { Type::Polysilicon, Qt::black    }
    };

    struct Border
    {
        QColor color;
        qreal width;
    };

    void setBorderColor(QColor);
    void setBorderWidth(qreal);

    LayerView(const QRectF&, Type);
    QLabel* info();

    // getters
    QRectF getRect();
    QRectF const getRect() const;

    virtual ~LayerView() = default;

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    Border m_border;
    QColor m_color;
};

#endif // LAYER_VIEW_H
