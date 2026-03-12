#include "scene_presenter.hpp"
#include "mappers.hpp"
#include "factories.hpp"
#include "view.hpp"

ScenePresenter::ScenePresenter(Context& context, SceneView& view, View* viewWidget) noexcept :
    m_context(context),
    m_view(view),
    m_viewWidget(viewWidget),
    m_strategy(nullptr),
    m_selectedComponent(nullptr),
    m_builder(nullptr)
{
    connect(&m_view, &SceneView::sceneClick, this, &ScenePresenter::handleSceneClick);
    connect(&m_view, &SceneView::sceneMouseMove, this, &ScenePresenter::handleMouseMove);
    connect(&m_view, &SceneView::sceneMouseRelease, this, &ScenePresenter::handleMouseRelease);
    connect(&m_view, &SceneView::mKeyPress, this, &ScenePresenter::handleMKeyPress);
    connect(this,    &ScenePresenter::drawRectPreview, &m_view, &SceneView::handleDrawRectPreview);
    connect(&m_view, &SceneView::deleteKeyPress, this, &ScenePresenter::handleDeleteKeyPress);
    connect(&m_view, &SceneView::undoPress, this, &ScenePresenter::handleUndoPress);
    connect(&m_view, &SceneView::redoPress, this, &ScenePresenter::handleRedoPress);
}

void ScenePresenter::handleDeleteKeyPress()
{
    if (m_selectedComponent)
    {
        fprintf(stderr, "ScenePresenter::handleDeleteKeyPress");

        if (!StyleModel().getStyle(typeid(*m_selectedComponent)).has_value())
        {
            fprintf(stderr, "optional is empty");
        }

        auto view = ViewFactory::create(m_selectedComponent->getShape(), *StyleModel().getStyle(typeid(*m_selectedComponent)));
        auto action = std::make_shared<RemoveLayerAction>(m_selectedComponent);
        auto undo = std::make_shared<CreateLayerAction>(m_selectedComponent, view);
        auto command = std::make_shared<RemoveLayerCommand>(action, undo);
        m_manager.execute(command, m_context, m_view);
    }
}

void ScenePresenter::handleMKeyPress()
{
    m_selectedComponent = std::make_unique<Metal<1>>(IdGenerator::generate(), nullptr);
    m_builder = std::make_unique<PolygonBuilder>();
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

void ScenePresenter::handleLayerPress(int id)
{
    auto modelId = m_context.m_viewToModel[id];
    if (auto it = std::ranges::find_if(m_context.m_layout.m_components, [&](const std::shared_ptr<AComponent>& component) {
        return component->id == modelId;
    }); it != m_context.m_layout.m_components.end())
    {
        if (auto l = dynamic_pointer_cast<Layer>(*it))
            m_selectedComponent = l;
    }
}

void ScenePresenter::onSelectedLayer(std::shared_ptr<Layer> selected)
{
    m_selectedComponent = selected;
    m_builder = std::make_unique<RectBuilder>();
    emit drawRectPreview();
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
    auto style = StyleModel().getStyle(typeid(*m_presenter.m_selectedComponent));

    if (!style.has_value())
    {
        fprintf(stderr, "optional is empty asdasdas");
    }

    // create views base on model data
    auto view = ViewFactory::create(shape.get(), *style);
    m_presenter.m_selectedComponent->setShape(std::move(shape));

    auto action = std::make_shared<CreateLayerAction>(m_presenter.m_selectedComponent, view);
    auto undoAction = std::make_shared<RemoveLayerAction>(m_presenter.m_selectedComponent);
    auto command = std::make_shared<CreateLayerCommand>(action, undoAction);

    m_presenter.m_manager.execute(command, m_presenter.m_context, m_presenter.m_view);

    if (auto layerView = dynamic_cast<LayerView*>(view))
    {
        QObject::connect(layerView, &LayerView::press, &m_presenter, &ScenePresenter::handleLayerPress);
        QObject::connect(layerView, &LayerView::moved, &m_presenter, &ScenePresenter::handleMoved);
    }
    m_presenter.m_builder = nullptr;
}

DoubleClickStrategy::DoubleClickStrategy(const QPointF& p, ScenePresenter& presenter) : p(p), m_presenter(presenter) { }

void DoubleClickStrategy::handle(const QPointF& p) { }

void ScenePresenter::handleUndoPress()
{
    fprintf(stderr, "ScenePresenter::handleUndoPress\n");
    m_manager.undo(m_view, m_context);
}

void ScenePresenter::handleRedoPress()
{
    fprintf(stderr, "ScenePresenter::handleRedoPress\n");   
    m_manager.redo(m_view, m_context);
}

void ScenePresenter::handleMoved(int id, const QPointF& prev, const QPointF& curr)
{
    auto modelId = m_context.m_viewToModel[id];

    for (const auto& component : m_context.m_layout.m_components)
    {
        if (auto layer = dynamic_cast<Layer*>(component.get()))
        {
            if (layer->id == modelId)
                m_selectedComponent = component;
        }
    }

    auto action = std::make_shared<MovedComponentAction>(m_selectedComponent, toPoint(curr));
    auto undoAction = std::make_shared<MovedComponentAction>(m_selectedComponent, toPoint(prev));
    auto command = std::make_shared<MovedComponentCommand>(action, undoAction);

    m_manager.execute(command, m_context, m_view);
}

void ScenePresenter::handleZoomIn()
{
    if (m_viewWidget) m_viewWidget->scale(1.2, 1.2);
}

void ScenePresenter::handleZoomOut()
{
    if (m_viewWidget) m_viewWidget->scale(1.0 / 1.2, 1.0 / 1.2);
}
















