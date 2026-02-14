#include "scene_presenter.hpp"
#include "mappers.hpp"

ScenePresenter::ScenePresenter(Context& context, SceneView& view) noexcept :
    m_context(context),
    m_view(view),
    m_strategy(nullptr),
    m_selectedLayer(nullptr),
    m_builder(nullptr)
{
    fprintf(stderr, "ScenePresenter\n");
    connect(&m_view, &SceneView::sceneClick, this, &ScenePresenter::handleSceneClick);
    connect(&m_view, &SceneView::sceneMouseMove, this, &ScenePresenter::handleMouseMove);
    connect(&m_view, &SceneView::sceneMouseRelease, this, &ScenePresenter::handleMouseRelease);
}

void ScenePresenter::setState(std::unique_ptr<IStrategy> state) { m_strategy = std::move(state); }

void ScenePresenter::handle(const QPointF& p) { m_strategy->handle(p); }

void ScenePresenter::handleSceneClick(const QPointF& p)
{
    m_strategy = std::make_unique<PressStrategy>(p, *this);
    m_strategy->handle(p);
}

void ScenePresenter::handleMouseMove(const QPointF& p)
{
    m_strategy = std::make_unique<MoveStrategy>(p, *this);
    m_strategy->handle(p);
}

void ScenePresenter::handleMouseRelease(const QPointF& p)
{
    m_strategy = std::make_unique<ReleaseStrategy>(p, *this);
    m_strategy->handle(p);
}

void ScenePresenter::onSelectedLayer(Layer* selected)
{
    m_selectedLayer = std::unique_ptr<Layer>(selected);
    m_builder = std::make_unique<RectBuilder>();
}

PressStrategy::PressStrategy(const QPointF& p, ScenePresenter& presenter) : p(p), m_presenter(presenter) { }

void PressStrategy::handle(const QPointF& p)
{
    if (m_presenter.m_builder != nullptr)
        m_presenter.m_builder->onPress(toPoint(p));
}

MoveStrategy::MoveStrategy(const QPointF& p, ScenePresenter& presenter) : p(p), m_presenter(presenter) { }

void MoveStrategy::handle(const QPointF& p)
{
    if (m_presenter.m_builder != nullptr)
        m_presenter.m_builder->onMove(toPoint(p));
}

ReleaseStrategy::ReleaseStrategy(const QPointF& p, ScenePresenter& presenter) : p(p), m_presenter(presenter) { }

void ReleaseStrategy::handle(const QPointF& p)
{
    if (!m_presenter.m_builder)
        return;

    // create shape via builder
    auto shape = m_presenter.m_builder->onRelease(toPoint(p)).build();

    // create views base on model data
    if (auto r = dynamic_cast<Rect*>(shape.get()); r != nullptr)
    {
        auto rect = toQRectF(*r);
        auto layerView = std::make_shared<LayerView>(rect, *LayerModel().getStyle(typeid(*m_presenter.m_selectedLayer)));
        m_presenter.m_selectedLayer->setShape(std::move(shape));

        auto action = std::make_shared<CreateLayerAction>(m_presenter.m_selectedLayer, layerView);
        auto undoAction = std::make_shared<RemoveLayerAction>(m_presenter.m_selectedLayer, layerView);
        auto command = std::make_shared<CreateLayerCommand>(action, undoAction);

        CommandManager manager;
        manager.execute(command, m_presenter.m_context, m_presenter.m_view);
    }
}

DoubleClickStrategy::DoubleClickStrategy(const QPointF& p, ScenePresenter& presenter) : p(p), m_presenter(presenter) { }

void DoubleClickStrategy::handle(const QPointF& p) { }
