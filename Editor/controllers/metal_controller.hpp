#ifndef METAL_CONTROLLER_HPP
#define METAL_CONTROLLER_HPP

#include <QObject>
#include <QPointF>
#include <memory>

#include "metal_view.hpp"
#include "metal.hpp"

class MetalController : public QObject
{
    Q_OBJECT

public:
    MetalController(Metal* metal, MetalView* metalView);
    void add(std::shared_ptr<QPointF>);
    void remove(std::shared_ptr<QPointF>);

public slots:
    void onMetalChanged();

private:
    Metal* m_metal;
    MetalView* m_view;
};


#endif // METAL_CONTROLLER_HPP
