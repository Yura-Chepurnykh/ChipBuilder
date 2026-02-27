#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <memory>
#include <stack>
#include "geometry.hpp"
#include "layers.hpp"
#include "scene_view.hpp"
#include "layer_view.hpp"
#include "context.hpp"
#include "mappers.hpp"

class IAction
{
public:
    virtual ~IAction() = default;
    virtual void execute(SceneView&, Context&) = 0;
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
    RemoveLayerCommand(std::shared_ptr<IAction>, std::shared_ptr<IAction>);

    void execute(SceneView&, Context&) override;
    void undo(SceneView&, Context&) override;

private:
    std::shared_ptr<IAction> m_action;
    std::shared_ptr<IAction> m_undoAction;
};

class CreateLayerAction final : public IAction
{
public:
    CreateLayerAction(std::shared_ptr<AComponent>, QGraphicsItem*);

    void execute(SceneView&, Context&) override;

private:
    std::shared_ptr<AComponent> m_layerModel;
    QGraphicsItem* m_layerView;
};

class RemoveLayerAction final : public IAction
{
public:
    RemoveLayerAction(std::shared_ptr<AComponent>);

    void execute(SceneView&, Context&) override;

private:
    std::shared_ptr<AComponent> m_model;
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

class MovedComponentCommand final : public ICommand
{
public:
    MovedComponentCommand(std::shared_ptr<IAction>, std::shared_ptr<IAction>);
    void execute(SceneView&, Context&) override;
    void undo(SceneView&, Context&) override;

private:
    std::shared_ptr<IAction> m_action;
    std::shared_ptr<IAction> m_undoAction;
};

class MovedComponentAction final : public IAction
{
public:
    MovedComponentAction(std::shared_ptr<AComponent>, const Point&);
    void execute(SceneView&, Context&) override;

private:
    Point m_toPoint;
    std::shared_ptr<AComponent> m_componentModel;
};

class RevertMovedComponentAction final : public IAction
{
public:
    RevertMovedComponentAction(std::shared_ptr<AComponent>, const Point&);
    void execute(SceneView&, Context&) override;

private:
    Point m_fromPoint;
    std::shared_ptr<AComponent> m_componentModel;
};

#endif // COMMANDS_HPP
