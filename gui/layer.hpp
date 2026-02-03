#ifndef LAYER_HPP
#define LAYER_HPP

#include <QGraphicsItem>
#include <QRectF>
#include <QHash>
#include <QColor>
#include <QPainter>
#include <QLabel>

class Layer : public QGraphicsItem
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

    Layer(const QRectF&, Type);
    QLabel* info();

    // getters
    QRectF getRect();
    QRectF const getRect() const;

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    Type m_type;
    QColor m_color;
    QRectF m_rect;
};

#endif // LAYER_HPP
