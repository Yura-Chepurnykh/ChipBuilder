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

class RemoveGroupCommand final : public ICommand
{
public:
    RemoveGroupCommand(std::vector<std::shared_ptr<IAction>> actions, std::vector<std::shared_ptr<IAction>> undoActions);

    void execute(SceneView&, Context&) override;
    void undo(SceneView&, Context&) override;

private:
    std::vector<std::shared_ptr<IAction>> m_actions;
    std::vector<std::shared_ptr<IAction>> m_undoActions;
};

class GroupCommand final : public ICommand
{
public:
    GroupCommand(std::shared_ptr<CircuitLayout> group, std::vector<std::shared_ptr<AComponent>> components);
    void execute(SceneView&, Context&) override;
    void undo(SceneView&, Context&) override;

private:
    std::shared_ptr<CircuitLayout> m_group;
    std::vector<std::shared_ptr<AComponent>> m_components;
};

// class MergeCommand final : public ICommand
// {
// public:
//     MergeCommand(std::shared_ptr<AComponent> merged, std::vector<std::shared_ptr<AComponent>> originals);
//     void execute(SceneView&, Context&) override;
//     void undo(SceneView&, Context&) override;

// private:
//     std::shared_ptr<AComponent> m_merged;
//     std::vector<std::shared_ptr<AComponent>> m_originals;
//     std::vector<int> m_viewIds;
// };

class UngroupCommand final : public ICommand
{
public:
    UngroupCommand(std::shared_ptr<CircuitLayout> group);
    void execute(SceneView&, Context&) override;
    void undo(SceneView&, Context&) override;

private:
    std::shared_ptr<CircuitLayout> m_group;
    std::vector<std::shared_ptr<AComponent>> m_components;
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

class ResizedComponentCommand final : public ICommand
{
public:
    ResizedComponentCommand(std::shared_ptr<IAction>, std::shared_ptr<IAction>);
    void execute(SceneView&, Context&) override;
    void undo(SceneView&, Context&) override;

private:
    std::shared_ptr<IAction> m_action;
    std::shared_ptr<IAction> m_undoAction;
};

class ResizedComponentAction final : public IAction
{
public:
    ResizedComponentAction(std::shared_ptr<AComponent>, const QRectF&);
    void execute(SceneView&, Context&) override;

private:
    QRectF m_toRect;
    std::shared_ptr<AComponent> m_componentModel;
};

class MovePolygonEdgeCommand final : public ICommand
{
public:
    MovePolygonEdgeCommand(int viewId, int edgeIndex, const Point& delta);
    void execute(SceneView&, Context&) override;
    void undo(SceneView&, Context&) override;

private:
    int m_viewId;
    int m_index; // Start vertex index of the edge
    Point m_delta;
};

#endif // COMMANDS_HPP
