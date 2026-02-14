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

CreateLayerAction::CreateLayerAction(std::shared_ptr<AComponent> layer, std::shared_ptr<LayerView> view) :
    m_layerModel(layer), m_layerView(view) { }

void CreateLayerAction::execute(SceneView& view, Context& context)
{
    LayerView* viewPtr = new LayerView(m_layerView->getRect(), m_layerView->getStyle());
    context.add(m_layerModel);
    view.addItem(viewPtr);
    view.update();
}

void CreateLayerAction::undo(SceneView& view, Context& context)
{
    context.remove(m_layerModel->id);
    view.removeItem(m_layerView.get());
    view.update();
}

RemoveLayerAction::RemoveLayerAction(std::shared_ptr<Layer> layer, std::shared_ptr<LayerView> view) :
    m_view(view), m_model(layer) { }

void RemoveLayerAction::execute(SceneView& view, Context& context)
{
    context.remove(m_model->id);
    view.removeItem(m_view.get());
    view.update();
}

void RemoveLayerAction::undo(SceneView& view, Context& context)
{
    context.add(m_model);
    view.addItem(m_view.get());
    view.update();
}


