#include "commands.hpp"

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

RemoveLayerAction::RemoveLayerAction(std::shared_ptr<Layer> layer) : m_model(layer) { }

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

    view.update();
}


