#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <memory>
#include <stack>
#include "geometry.hpp"
#include "layers.hpp"
#include "scene_view.hpp"
#include "layer_view.hpp"
#include "context.hpp"

class IAction
{
public:
    virtual ~IAction() = default;
    virtual void execute(SceneView&, Context&) = 0;
    virtual void undo(SceneView&, Context&) = 0;
};

class ICommand
{
public:
    virtual ~ICommand() = default;
    virtual void execute(SceneView&, Context&) = 0;
    virtual void undo(SceneView&, Context&) = 0;
};

class CreateLayerCommand final : public ICommand
{
public:
    CreateLayerCommand(std::shared_ptr<IAction>, std::shared_ptr<IAction>);

    void execute(SceneView&, Context&) override;
    void undo(SceneView&, Context&) override;

private:
    std::shared_ptr<IAction> m_action;
    std::shared_ptr<IAction> m_undoAction;
};

class RemoveLayerCommand final : public ICommand
{
public:
    RemoveLayerCommand(std::shared_ptr<AComponent>, QGraphicsItem*);

    void execute(SceneView&, Context&) override;
    void undo(SceneView&, Context&) override;

private:
    std::shared_ptr<AComponent> m_model;
    QGraphicsItem* m_view;
};

class CreateLayerAction final : public IAction
{
public:
    CreateLayerAction(std::shared_ptr<AComponent>, QGraphicsItem*);

    void execute(SceneView&, Context&) override;
    void undo(SceneView&, Context&) override;

private:
    std::shared_ptr<AComponent> m_layerModel;
    QGraphicsItem* m_layerView;
};

class RemoveLayerAction final : public IAction
{
public:
    RemoveLayerAction(std::shared_ptr<Layer>, QGraphicsItem*);

    void execute(SceneView&, Context&) override;
    void undo(SceneView&, Context&) override;

private:
    std::shared_ptr<AComponent> m_model;
    QGraphicsItem* m_view;
};

class CommandManager
{
public:
    void execute(std::shared_ptr<ICommand>, Context&, SceneView&);
    void undo(SceneView&, Context&);
    void redo(SceneView&, Context&);

private:
    std::stack<std::shared_ptr<ICommand>> m_undoHistory;
    std::stack<std::shared_ptr<ICommand>> m_redoHistory;
};

#endif // COMMANDS_HPP
