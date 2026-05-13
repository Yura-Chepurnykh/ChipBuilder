#include "commands.hpp"
#include "factories.hpp"

void CommandManager::execute(std::shared_ptr<ICommand> command, Context& context, SceneView& scene)
{
    command->execute(scene, context);
    m_undoHistory.push(command);

    if (!m_redoHistory.empty())
        m_redoHistory.pop();
}

void CommandManager::undo(SceneView& scene, Context& context)
{
    if (!m_undoHistory.empty())
    {
        auto command = std::move(m_undoHistory.top());
        command->undo(scene, context);
        m_redoHistory.push(command);
        m_undoHistory.pop();
    }
}

void CommandManager::redo(SceneView& scene, Context& context)
{
    if (!m_redoHistory.empty())
    {
        auto command = std::move(m_redoHistory.top());
        command->execute(scene, context);
        m_undoHistory.push(command);
        m_redoHistory.pop();
    }
}

CreateLayerCommand::CreateLayerCommand(std::shared_ptr<IAction> action, std::shared_ptr<IAction> undoAction) :
    m_action(std::move(action)), m_undoAction(std::move(undoAction)) { }

void CreateLayerCommand::execute(SceneView& view, Context& context)
{
    if (m_action)
        m_action->execute(view, context);
}

void CreateLayerCommand::undo(SceneView& view, Context& context)
{
    if (m_undoAction)
        m_undoAction->execute(view, context);
}

CreateLayerAction::CreateLayerAction(std::shared_ptr<AComponent> layer, QGraphicsItem* view) :
    m_layerModel(layer), m_layerView(view) { }

void CreateLayerAction::execute(SceneView& view, Context& context)
{
    // QGraphicsItem* viewPtr = new LayerView(m_layerView->getRect(), m_layerView->getStyle());
    if (auto layer = dynamic_cast<LayerView*>(m_layerView); layer != nullptr)
    {
        layer->zLevel = m_layerModel->zLevel;
        layer->setZValue(m_layerModel->zLevel);
        context.m_modelToView[m_layerModel->id] = layer->id;
        context.m_viewToModel[layer->id] = m_layerModel->id;
        context.m_layout.add(m_layerModel);
        view.addItem(m_layerView);
        view.update();
    }
    if (auto layer = dynamic_cast<MetalView*>(m_layerView); layer != nullptr)
    {
        context.m_modelToView[m_layerModel->id] = layer->id;
        context.m_viewToModel[layer->id] = m_layerModel->id;
        context.m_layout.add(m_layerModel);
        view.addItem(m_layerView);
        view.update();
    }
}

RemoveLayerCommand::RemoveLayerCommand(std::shared_ptr<IAction> action, std::shared_ptr<IAction> undoAction) :
    m_action(std::move(action)), m_undoAction(std::move(undoAction)) { }

void RemoveLayerCommand::execute(SceneView& view, Context& context)
{
    if (m_action)
        m_action->execute(view, context);
}

void RemoveLayerCommand::undo(SceneView& view, Context& context)
{
    if (m_undoAction)
        m_undoAction->execute(view, context);
}

RemoveLayerAction::RemoveLayerAction(std::shared_ptr<AComponent> layer) : m_model(layer) { }

void RemoveLayerAction::execute(SceneView& view, Context& context)
{
    context.m_layout.remove(m_model->id);

    auto id = context.m_modelToView[m_model->id];

    for (auto& item : view.items())
    {
        if (auto layer = dynamic_cast<LayerView*>(item); layer != nullptr)
        {
            if (layer->id == id)
                view.removeItem(layer);
        }
    }

    for (auto& item : view.items())
    {
        if (auto layer = dynamic_cast<MetalView*>(item); layer != nullptr)
        {
            if (layer->id == id)
                view.removeItem(layer);
        }
    }

    view.update();
}

RemoveGroupCommand::RemoveGroupCommand(std::vector<std::shared_ptr<IAction>> actions, std::vector<std::shared_ptr<IAction>> undoActions) :
    m_actions(std::move(actions)), m_undoActions(std::move(undoActions)) { }

void RemoveGroupCommand::execute(SceneView& view, Context& context)
{
    for (auto& action : m_actions)
        if (action) action->execute(view, context);
}

void RemoveGroupCommand::undo(SceneView& view, Context& context)
{
    for (auto& action : m_undoActions)
        if (action) action->execute(view, context);
}

GroupCommand::GroupCommand(std::shared_ptr<CircuitLayout> group, std::vector<std::shared_ptr<AComponent>> components) :
    m_group(group), m_components(std::move(components)) { }

void GroupCommand::execute(SceneView&, Context& context)
{
    for (const auto& component : m_components)
    {
        context.m_layout.remove(component->id);
        m_group->add(component);
    }
    context.m_layout.add(m_group);
}

void GroupCommand::undo(SceneView&, Context& context)
{
    context.m_layout.remove(m_group->id);
    for (const auto& component : m_components)
    {
        context.m_layout.add(component);
    }
}

// MergeCommand::MergeCommand(std::shared_ptr<AComponent> merged, std::vector<std::shared_ptr<AComponent>> originals) :
//     m_merged(merged), m_originals(std::move(originals)) { }

// void MergeCommand::execute(SceneView& view, Context& context)
// {
//     for (const auto& original : m_originals) {
//         int viewId = context.m_modelToView[original->id];
//         m_viewIds.push_back(viewId);

//         for (auto* item : view.items()) {
//             if (auto* layer = dynamic_cast<LayerView*>(item)) {
//                 if (layer->id == viewId) view.removeItem(layer);
//             } else if (auto* metal = dynamic_cast<MetalView*>(item)) {
//                 if (metal->id == viewId) view.removeItem(metal);
//             }
//         }
//         context.m_layout.remove(original->id);
//     }

//     auto style = StyleModel().getStyle(typeid(*m_merged));
//     auto viewItem = ViewFactory::create(m_merged->getShape(), *style, QString::fromStdString(m_merged->name()));
//     view.addItem(viewItem);

//     int newViewId = -1;
//     if (auto* layer = dynamic_cast<LayerView*>(viewItem)) newViewId = layer->id;
//     else if (auto* metal = dynamic_cast<MetalView*>(viewItem)) newViewId = metal->id;

//     context.m_layout.add(m_merged);
//     context.m_modelToView[m_merged->id] = newViewId;
//     context.m_viewToModel[newViewId] = m_merged->id;
//     view.update();
// }

// void MergeCommand::undo(SceneView& view, Context& context)
// {
//     int mergedViewId = context.m_modelToView[m_merged->id];
//     for (auto* item : view.items()) {
//         if (auto* layer = dynamic_cast<LayerView*>(item)) {
//             if (layer->id == mergedViewId) view.removeItem(layer);
//         } else if (auto* metal = dynamic_cast<MetalView*>(item)) {
//             if (metal->id == mergedViewId) view.removeItem(metal);
//         }
//     }
//     context.m_layout.remove(m_merged->id);

//     for (size_t i = 0; i < m_originals.size(); ++i) {
//         auto original = m_originals[i];
//         context.m_layout.add(original);

//         auto style = StyleModel().getStyle(typeid(*original));
//         auto viewItem = ViewFactory::create(original->getShape(), *style, QString::fromStdString(original->name()));
//         view.addItem(viewItem);

//         int newViewId = -1;
//         if (auto* layer = dynamic_cast<LayerView*>(viewItem)) {
//             newViewId = layer->id;
//             layer->id = m_viewIds[i]; // Restore old ID
//         } else if (auto* metal = dynamic_cast<MetalView*>(viewItem)) {
//             newViewId = metal->id;
//             metal->id = m_viewIds[i]; // Restore old ID
//         }

//         context.m_modelToView[original->id] = m_viewIds[i];
//         context.m_viewToModel[m_viewIds[i]] = original->id;
//     }
//     view.update();
// }

UngroupCommand::UngroupCommand(std::shared_ptr<CircuitLayout> group) :
    m_group(group), m_components(group->m_components) { }

void UngroupCommand::execute(SceneView&, Context& context)
{
    context.m_layout.remove(m_group->id);
    for (const auto& component : m_components)
    {
        context.m_layout.add(component);
    }
}

void UngroupCommand::undo(SceneView&, Context& context)
{
    for (const auto& component : m_components)
    {
        context.m_layout.remove(component->id);
    }
    context.m_layout.add(m_group);
}


MovedComponentCommand::MovedComponentCommand(std::shared_ptr<IAction> action, std::shared_ptr<IAction> undoAction) :
    m_action(action), m_undoAction(undoAction)
{

}

void MovedComponentCommand::execute(SceneView& view, Context& context)
{
    m_action->execute(view, context);
}

void MovedComponentCommand::undo(SceneView& view, Context& context)
{
    m_undoAction->execute(view, context);
}

MovedComponentAction::MovedComponentAction(std::shared_ptr<AComponent> component, const Point& p) : m_componentModel(component), m_toPoint(p) { }

void MovedComponentAction::execute(SceneView& view, Context& context)
{
    auto viewId = context.m_modelToView[m_componentModel->id];

    for (const auto& item : view.items())
    {
        int itemId = -1;
        if (auto* lv = dynamic_cast<LayerView*>(item)) itemId = lv->id;
        else if (auto* mv = dynamic_cast<MetalView*>(item)) itemId = mv->id;

        if (itemId == viewId)
        {
            item->setPos(toQPointF(m_toPoint));
            break;
        }
    }

    m_componentModel->move(m_toPoint);
}

RevertMovedComponentAction::RevertMovedComponentAction(std::shared_ptr<AComponent> component, const Point& p) :
    m_componentModel(component), m_fromPoint(p) { }

void RevertMovedComponentAction::execute(SceneView& view, Context& context)
{
    auto viewId = context.m_modelToView[m_componentModel->id];

    for (const auto& item : view.items())
    {
        int itemId = -1;
        if (auto* lv = dynamic_cast<LayerView*>(item)) itemId = lv->id;
        else if (auto* mv = dynamic_cast<MetalView*>(item)) itemId = mv->id;

        if (itemId == viewId)
        {
            item->setPos(toQPointF(m_fromPoint));
            break;
        }
    }

    m_componentModel->move(m_fromPoint);
}

ResizedComponentCommand::ResizedComponentCommand(std::shared_ptr<IAction> action, std::shared_ptr<IAction> undoAction) :
    m_action(action), m_undoAction(undoAction) { }

void ResizedComponentCommand::execute(SceneView& view, Context& context)
{
    m_action->execute(view, context);
}

void ResizedComponentCommand::undo(SceneView& view, Context& context)
{
    m_undoAction->execute(view, context);
}

ResizedComponentAction::ResizedComponentAction(std::shared_ptr<AComponent> component, const QRectF& rect) :
    m_componentModel(component), m_toRect(rect) { }

void ResizedComponentAction::execute(SceneView& view, Context& context)
{
    auto viewId = context.m_modelToView[m_componentModel->id];

    for (const auto& item : view.items())
    {
        if (auto layerView = dynamic_cast<LayerView*>(item))
        {
            if (layerView->id == viewId)
            {
                // To avoid jumping, we set the position to the top-left corner
                // and make the internal rectangle relative to (0,0)
                layerView->setPos(m_toRect.topLeft());
                layerView->setRect(0, 0, m_toRect.width(), m_toRect.height());
                layerView->update();
                break;
            }
        }
    }

    if (auto shape = m_componentModel->getShape())
    {
        if (auto rect = dynamic_cast<Rect*>(shape))
        {
            // Normalize coordinates for the model
            rect->point = Point(-1, m_toRect.x(), m_toRect.y());
            rect->width = std::abs(m_toRect.width());
            rect->height = std::abs(m_toRect.height());
        }
    }
}
