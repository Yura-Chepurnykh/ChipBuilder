#include "scene_presenter.hpp"
#include "mappers.hpp"
#include "factories.hpp"
#include "union_handler.hpp"
#include "metal_view.hpp"
#include "drc.hpp"
#include <unordered_set>

ScenePresenter::ScenePresenter(Context& context, SceneView& view, QListWidget* drcSidebar)  :
    m_context(context),
    m_view(view),
    m_drcSidebar(drcSidebar),
    m_strategy(nullptr),
    m_selectedComponent(nullptr),
    m_builder(nullptr)
{
    connect(&m_view, &SceneView::sceneClick, this, &ScenePresenter::handleSceneClick);
    connect(&m_view, &SceneView::sceneMouseMove, this, &ScenePresenter::handleMouseMove);
    connect(&m_view, &SceneView::sceneMouseRelease, this, &ScenePresenter::handleMouseRelease);
    connect(&m_view, &SceneView::sceneMouseDoubleClick, this, &ScenePresenter::handleMouseDoubleClick);
    connect(&m_view, &SceneView::mKeyPress, this, &ScenePresenter::handleMKeyPress);
    connect(this,    &ScenePresenter::drawRectPreview, &m_view, &SceneView::handleDrawRectPreview);
    connect(this,    &ScenePresenter::drawPolygonPreview, &m_view, &SceneView::handleDrawPolygonPreview);
    connect(&m_view, &SceneView::deleteKeyPress, this, &ScenePresenter::handleDeleteKeyPress);
    connect(&m_view, &SceneView::undoPress, this, &ScenePresenter::handleUndoPress);
    connect(&m_view, &SceneView::redoPress, this, &ScenePresenter::handleRedoPress);
}

void ScenePresenter::handleDeleteKeyPress()
{
    auto selectedItems = m_view.selectedItems();
    if (selectedItems.isEmpty() && m_selectedComponent)
    {
        auto style = StyleModel().getStyle(typeid(*m_selectedComponent));
        auto view = ViewFactory::create(m_selectedComponent->getShape(), *style, QString::fromStdString(m_selectedComponent->name()));
        auto action = std::make_shared<RemoveLayerAction>(m_selectedComponent);
        auto undo = std::make_shared<CreateLayerAction>(m_selectedComponent, view);
        auto command = std::make_shared<RemoveLayerCommand>(action, undo);
        m_manager.execute(command, m_context, m_view);
        m_selectedComponent = nullptr;
    }
    else
    {
        std::vector<std::shared_ptr<IAction>> actions;
        std::vector<std::shared_ptr<IAction>> undoActions;

        for (auto* item : selectedItems)
        {
            int viewId = -1;
            if (auto* layer = dynamic_cast<LayerView*>(item)) viewId = layer->id;
            else if (auto* metal = dynamic_cast<MetalView*>(item)) viewId = metal->id;

            if (viewId != -1)
            {
                auto modelId = m_context.m_viewToModel[viewId];
                auto it = std::ranges::find_if(m_context.m_layout.m_components, [&](const std::shared_ptr<AComponent>& c) {
                    return c->id == modelId;
                });

                if (it != m_context.m_layout.m_components.end())
                {
                    auto component = *it;
                    auto style = StyleModel().getStyle(typeid(*component));
                    auto view = ViewFactory::create(component->getShape(), *style, QString::fromStdString(component->name()));
                    
                    actions.push_back(std::make_shared<RemoveLayerAction>(component));
                    undoActions.push_back(std::make_shared<CreateLayerAction>(component, view));
                    
                    if (m_selectedComponent == component) m_selectedComponent = nullptr;
                }
            }
        }

        if (!actions.empty())
        {
            auto groupCommand = std::make_shared<RemoveGroupCommand>(std::move(actions), std::move(undoActions));
            m_manager.execute(groupCommand, m_context, m_view);
        }
    }
    syncDRC();
}

void ScenePresenter::handleMKeyPress()
{
    for (auto* view : m_view.views()) view->setDragMode(QGraphicsView::NoDrag);
    m_selectedComponent = std::make_shared<Metal1>(IdGenerator::generate(), 0, nullptr);
    m_builder = std::make_unique<RectBuilder>();
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

void ScenePresenter::handleMouseDoubleClick(const QPointF& p)
{
    m_strategy = std::make_unique<DoubleClickStrategy>(p, *this);
    m_strategy->handle(p);
}

void ScenePresenter::handleLayerPress(int id)
{
    auto modelId = m_context.m_viewToModel[id];
    for (const auto& component : m_context.m_layout.m_components)
    {
        if (component->id == modelId)
            m_selectedComponent = component;
    }
}

void ScenePresenter::onSelectedLayer(std::shared_ptr<Layer> selected)
{
    for (auto* view : m_view.views()) view->setDragMode(QGraphicsView::NoDrag);
    m_selectedComponent = selected;
    if (dynamic_cast<Metal1*>(selected.get()))
    {
        m_builder = std::make_unique<PolygonBuilder>();
        emit drawPolygonPreview();
    }
    else
    {
        m_builder = std::make_unique<RectBuilder>();
        emit drawRectPreview();
    }
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

void ScenePresenter::bindView(QGraphicsItem* view)
{
    qDebug() << "ScenePresenter::bindView called for item:" << view;
    if (auto layerView = dynamic_cast<LayerView*>(view))
    {
        QObject::connect(layerView, &LayerView::press, this, &ScenePresenter::handleLayerPress);
        QObject::connect(layerView, &LayerView::moved, this, &ScenePresenter::handleMoved);
        QObject::connect(layerView, &LayerView::resized, this, &ScenePresenter::handleResized);
        QObject::connect(layerView, &LayerView::geometryChanged, this, &ScenePresenter::handleGeometryChanged);
        QObject::connect(layerView, &LayerView::raiseRequested, this, &ScenePresenter::handleRaiseLayer);
        QObject::connect(layerView, &LayerView::lowerRequested, this, &ScenePresenter::handleLowerLayer);
        QObject::connect(layerView, &LayerView::setLevelRequested, this, &ScenePresenter::handleSetLayerLevel);
        qDebug() << "Signals connected for LayerView id:" << layerView->id;
    }
    else if (auto metalView = dynamic_cast<MetalView*>(view))
    {
        QObject::connect(metalView, &MetalView::geometryChanged, this, &ScenePresenter::handleMetalGeometryChanged);
        qDebug() << "Signals connected for MetalView id:" << metalView->id;
    }
}

void ReleaseStrategy::handle(const QPointF& p)
{
    if (!m_presenter.m_builder) return;

    if (dynamic_cast<PolygonBuilder*>(m_presenter.m_builder.get()))
    {
        m_presenter.m_builder->onRelease(toPoint(p));
        return;
    }

    auto shape = m_presenter.m_builder->onRelease(toPoint(p)).build();
    if (auto r = dynamic_cast<Rect*>(shape.get())) {
        if (r->width < 0) { r->point.x += r->width; r->width = -r->width; }
        if (r->height < 0) { r->point.y += r->height; r->height = -r->height; }
    }
    auto style = StyleModel().getStyle(typeid(*m_presenter.m_selectedComponent));
    auto view = ViewFactory::create(shape.get(), *style, QString::fromStdString(m_presenter.m_selectedComponent->name()));
    m_presenter.m_selectedComponent->setShape(std::move(shape));

    auto action = std::make_shared<CreateLayerAction>(m_presenter.m_selectedComponent, view);
    auto undoAction = std::make_shared<RemoveLayerAction>(m_presenter.m_selectedComponent);
    auto command = std::make_shared<CreateLayerCommand>(action, undoAction);

    m_presenter.m_manager.execute(command, m_presenter.m_context, m_presenter.m_view);

    m_presenter.bindView(view);
    m_presenter.syncDRC();
    
    m_presenter.m_builder = nullptr;
}

DoubleClickStrategy::DoubleClickStrategy(const QPointF& p, ScenePresenter& presenter) : p(p), m_presenter(presenter) { }
void DoubleClickStrategy::handle(const QPointF& p)
{
    if (!m_presenter.m_builder) return;

    auto shape = m_presenter.m_builder->onDouble(toPoint(p)).build();
    if (auto poly = dynamic_cast<PolygonShape*>(shape.get()))
    {
        if (poly->m_points.size() < 2) return;
    }

    auto style = StyleModel().getStyle(typeid(*m_presenter.m_selectedComponent));
    auto view = ViewFactory::create(shape.get(), *style, QString::fromStdString(m_presenter.m_selectedComponent->name()));
    m_presenter.m_selectedComponent->setShape(std::move(shape));

    auto action = std::make_shared<CreateLayerAction>(m_presenter.m_selectedComponent, view);
    auto undoAction = std::make_shared<RemoveLayerAction>(m_presenter.m_selectedComponent);
    auto command = std::make_shared<CreateLayerCommand>(action, undoAction);

    m_presenter.m_manager.execute(command, m_presenter.m_context, m_presenter.m_view);

    m_presenter.bindView(view);
    m_presenter.syncDRC();
    
    m_presenter.m_builder = nullptr;
}

void ScenePresenter::handleUndoPress()
{
    m_manager.undo(m_view, m_context);
    syncDRC();
}

void ScenePresenter::handleRedoPress()
{
    m_manager.redo(m_view, m_context);
    syncDRC();
}

void ScenePresenter::syncDRC()
{
    if (m_drcSidebar) m_drcSidebar->clear();

    // 1. Reset all views
    for (auto* item : m_view.items()) {
        if (auto* layer = dynamic_cast<LayerView*>(item)) {
            layer->setDRCViolated(false);
        } else if (auto* metal = dynamic_cast<MetalView*>(item)) {
            metal->setDRCViolated(false);
        }
    }

    // 2. Run DRC
    auto violations = runDRC(m_context.m_layout);

    // 3. Highlight violations and show messages
    for (const auto& v : violations) {
        if (m_drcSidebar) {
            auto* item = new QListWidgetItem(QString::fromStdString(v.toString()));
            item->setForeground(Qt::red);
            m_drcSidebar->addItem(item);
        }
        
        auto highlight = [&](unsigned int modelId) {
            if (modelId == 0) return;
            auto it = m_context.m_modelToView.find(modelId);
            if (it != m_context.m_modelToView.end()) {
                int viewId = it->second;
                for (auto* item : m_view.items()) {
                    if (auto* layer = dynamic_cast<LayerView*>(item)) {
                        if (layer->id == viewId) layer->setDRCViolated(true);
                    } else if (auto* metal = dynamic_cast<MetalView*>(item)) {
                        if (metal->id == viewId) metal->setDRCViolated(true);
                    }
                }
            }
        };

        highlight(v.componentIdA);
        highlight(v.componentIdB);
    }
}

void ScenePresenter::handleRectSelectionTriggered()
{
    for (auto* view : m_view.views()) view->setDragMode(QGraphicsView::NoDrag);
    m_view.m_isRectSelection = true;
    m_view.m_isLassoSelection = false;
    m_view.m_isDrawRect = false;
    m_view.m_isDrawPolygon = false;
    m_selectedComponent = nullptr;
    m_builder = nullptr;
}

void ScenePresenter::handleLassoSelectionTriggered()
{
    for (auto* view : m_view.views()) view->setDragMode(QGraphicsView::NoDrag);
    m_view.m_isRectSelection = false;
    m_view.m_isLassoSelection = true;
    m_view.m_isDrawRect = false;
    m_view.m_isDrawPolygon = false;
    m_selectedComponent = nullptr;
    m_builder = nullptr;
}

void ScenePresenter::handlePanningTriggered()
{
    for (auto* view : m_view.views()) view->setDragMode(QGraphicsView::ScrollHandDrag);
    m_view.m_isRectSelection = false;
    m_view.m_isLassoSelection = false;
    m_view.m_isDrawRect = false;
    m_view.m_isDrawPolygon = false;
    m_selectedComponent = nullptr;
    m_builder = nullptr;
}

void ScenePresenter::handleZoomIn()
{
    for (auto* view : m_view.views()) view->scale(1.1, 1.1);
}

void ScenePresenter::handleZoomOut()
{
    for (auto* view : m_view.views()) view->scale(0.9, 0.9);
}

void ScenePresenter::handleGroupUngroup()
{
    auto selectedItems = m_view.selectedItems();
    if (selectedItems.isEmpty()) return;
    
    std::vector<std::shared_ptr<AComponent>> components;
    for (auto* item : selectedItems)
    {
        int viewId = -1;
        if (auto* layer = dynamic_cast<LayerView*>(item)) viewId = layer->id;
        else if (auto* metal = dynamic_cast<MetalView*>(item)) viewId = metal->id;

        if (viewId != -1)
        {
            auto modelId = m_context.m_viewToModel[viewId];
            auto it = std::ranges::find_if(m_context.m_layout.m_components, [&](const std::shared_ptr<AComponent>& c) {
                return c->id == modelId;
            });
            if (it != m_context.m_layout.m_components.end()) components.push_back(*it);
        }
    }
}

void ScenePresenter::handleRaiseLayer(int id)
{
    qDebug() << "ScenePresenter::handleRaiseLayer requested for view id:" << id;
    auto modelId = m_context.m_viewToModel[id];
    for (auto& component : m_context.m_layout.m_components)
    {
        if (component->id == modelId)
        {
            component->zLevel++;
            qDebug() << "Model ID" << modelId << "new zLevel:" << component->zLevel;
            for (auto* item : m_view.items())
            {
                if (auto* layer = dynamic_cast<LayerView*>(item))
                {
                    if (layer->id == id)
                    {
                        layer->zLevel = component->zLevel;
                        layer->setZValue(component->zLevel);
                        layer->update();
                        qDebug() << "View updated: zValue =" << layer->zValue();
                    }
                }
            }
            break;
        }
    }
    m_view.update();
}

void ScenePresenter::handleLowerLayer(int id)
{
    qDebug() << "ScenePresenter::handleLowerLayer requested for view id:" << id;
    auto modelId = m_context.m_viewToModel[id];
    for (auto& component : m_context.m_layout.m_components)
    {
        if (component->id == modelId)
        {
            component->zLevel--;
            qDebug() << "Model ID" << modelId << "new zLevel:" << component->zLevel;
            for (auto* item : m_view.items())
            {
                if (auto* layer = dynamic_cast<LayerView*>(item))
                {
                    if (layer->id == id)
                    {
                        layer->zLevel = component->zLevel;
                        layer->setZValue(component->zLevel);
                        layer->update();
                    }
                }
            }
            break;
        }
    }
    m_view.update();
}

void ScenePresenter::handleSetLayerLevel(int id, int level)
{
    qDebug() << "ScenePresenter::handleSetLayerLevel requested for view id:" << id << "to level:" << level;
    auto modelId = m_context.m_viewToModel[id];
    for (auto& component : m_context.m_layout.m_components)
    {
        if (component->id == modelId)
        {
            component->zLevel = level;
            for (auto* item : m_view.items())
            {
                if (auto* layer = dynamic_cast<LayerView*>(item))
                {
                    if (layer->id == id)
                    {
                        layer->zLevel = component->zLevel;
                        layer->setZValue(component->zLevel);
                        layer->update();
                    }
                }
            }
            break;
        }
    }
    m_view.update();
}

void ScenePresenter::handleMoved(int id, const QPointF& prev, const QPointF& curr)
{
    auto modelId = m_context.m_viewToModel[id];
    for (const auto& component : m_context.m_layout.m_components)
    {
        if (auto layer = dynamic_cast<Layer*>(component.get()))
            if (layer->id == modelId) m_selectedComponent = component;
    }

    auto undoAction = std::make_shared<RevertMovedComponentAction>(m_selectedComponent, toPoint(prev));
    auto action = std::make_shared<MovedComponentAction>(m_selectedComponent, toPoint(curr));
    auto command = std::make_shared<MovedComponentCommand>(action, undoAction);
    m_manager.execute(command, m_context, m_view);
    syncDRC();
}

void ScenePresenter::handleResized(int id, const QRectF& prev, const QRectF& curr)
{
    QRectF normCurr = curr.normalized();
    QRectF normPrev = prev.normalized();

    auto modelId = m_context.m_viewToModel[id];
    std::shared_ptr<AComponent> targetComponent = nullptr;
    for (const auto& component : m_context.m_layout.m_components)
    {
        if (component->id == modelId) {
            targetComponent = component;
            break;
        }
    }

    if (targetComponent) {
        auto action = std::make_shared<ResizedComponentAction>(targetComponent, normCurr);
        auto undoAction = std::make_shared<ResizedComponentAction>(targetComponent, normPrev);
        auto command = std::make_shared<ResizedComponentCommand>(action, undoAction);
        m_manager.execute(command, m_context, m_view);
        syncDRC();
    }
}

void ScenePresenter::handleMetalGeometryChanged(int id)
{
    auto modelId = m_context.m_viewToModel[id];
    
    MetalView* metalView = nullptr;
    for (auto* item : m_view.items())
    {
        if (auto* mv = dynamic_cast<MetalView*>(item))
        {
            if (mv->id == id)
            {
                metalView = mv;
                break;
            }
        }
    }
    
    if (!metalView) return;

    for (auto& component : m_context.m_layout.m_components)
    {
        if (component->id == modelId)
        {
            if (auto poly = dynamic_cast<PolygonShape*>(component->getShape()))
            {
                poly->m_points.clear();
                QPointF offset = metalView->pos();
                for (const auto& p : metalView->getPath())
                {
                    poly->m_points.push_back(Point(-1, p->x() + offset.x(), p->y() + offset.y()));
                }
            }
            break;
        }
    }
    syncDRC();
}

void ScenePresenter::handleGeometryChanged(int id, const QRectF& curr)
{
    QRectF normCurr = curr.normalized();
    auto modelId = m_context.m_viewToModel[id];
    for (auto& component : m_context.m_layout.m_components)
    {
        if (component->id == modelId)
        {
            if (auto r = dynamic_cast<Rect*>(component->getShape()))
            {
                r->point.x = normCurr.x();
                r->point.y = normCurr.y();
                r->width = normCurr.width();
                r->height = normCurr.height();
            }
            break;
        }
    }
    syncDRC();
}

ScenePresenter::~ScenePresenter()
{
}
