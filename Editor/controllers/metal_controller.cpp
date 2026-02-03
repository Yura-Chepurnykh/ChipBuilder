#include "metal_controller.hpp"

MetalController::MetalController(Metal* metal, MetalView* view) : m_metal(metal), m_view(view)
{
    connect(metal, &Metal::changed, this, &MetalController::onMetalChanged);
}

void MetalController::onMetalChanged()
{
    m_view->update();
}

void MetalController::add(std::shared_ptr<QPointF> p)
{
    m_metal->add(p);
}

void MetalController::remove(std::shared_ptr<QPointF> p)
{
    m_metal->remove(p);
}
