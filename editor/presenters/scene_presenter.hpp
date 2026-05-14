#ifndef SCENE_PRESENTER_H
#define SCENE_PRESENTER_H

#include <memory>
#include <QDebug>
#include <unordered_map>
#include "layers.hpp"
#include "scene_view.hpp"
#include "layer_view.hpp"
#include "scene_model.hpp"
#include "commands.hpp"
#include "layer_model.hpp"
#include "builder.hpp"
#include "id_generator.hpp"
#include <QObject>
#include <QGraphicsItemGroup>
#include <QListWidget>

class ScenePresenter;

class IStrategy
{
public:
    virtual void handle(const QPointF&) = 0;
    virtual ~IStrategy() = default;
};

class PressStrategy final : public IStrategy
{
public:
    PressStrategy(const QPointF&, ScenePresenter&);
    void handle(const QPointF&) override;

private:
    QPointF p;
    ScenePresenter& m_presenter;
};

class MoveStrategy final : public IStrategy
{
public:
    MoveStrategy(const QPointF&, ScenePresenter&);
    void handle(const QPointF&) override;

private:
    QPointF p;
    ScenePresenter& m_presenter;
};

class ReleaseStrategy final : public IStrategy
{
public:
    ReleaseStrategy(const QPointF&, ScenePresenter&);
    void handle(const QPointF&) override;

private:
    QPointF p;
    ScenePresenter& m_presenter;
};

class DoubleClickStrategy final : public IStrategy
{
public:
    DoubleClickStrategy(const QPointF&, ScenePresenter&);
    void handle(const QPointF&) override;

private:
    QPointF p;
    ScenePresenter& m_presenter;
};

class ScenePresenter : public QObject
{
    Q_OBJECT

public:
    ScenePresenter(Context& context, SceneView& view, QListWidget* drcSidebar);
    ~ScenePresenter();
    void setState(std::unique_ptr<IStrategy>);
    void handle(const QPointF&);

signals:
    void drawRectPreview();
    void drawPolygonPreview();

public slots:
    void handleLayerPress(int);
    void handleSceneClick(const QPointF&);
    void handleMouseMove(const QPointF&);
    void handleMouseRelease(const QPointF&);
    void handleMouseDoubleClick(const QPointF&);
    void onSelectedLayer(std::shared_ptr<Layer>);

    void handleMoved(int, const QPointF&, const QPointF&);
    void handleResized(int, const QRectF&, const QRectF&);
    void handleGeometryChanged(int, const QRectF&);
    void handleMetalGeometryChanged(int);

public slots:
    void handleMKeyPress();
    void handleDeleteKeyPress();
    void handleUndoPress();
    void handleRedoPress();

    void handleRaiseLayer(int);
    void handleLowerLayer(int);
    void handleSetLayerLevel(int, int);

    void bindView(QGraphicsItem*);

    void syncDRC();

    void handleRectSelectionTriggered();
    void handleLassoSelectionTriggered();
    void handlePanningTriggered();

    void handleZoomIn();
    void handleZoomOut();
    void handleGroupUngroup();

public:
    Context& m_context;
    SceneView& m_view;
    QListWidget* m_drcSidebar;
    std::unique_ptr<IStrategy> m_strategy;
    std::shared_ptr<AComponent> m_selectedComponent;
    std::unique_ptr<IShapeBuilder> m_builder;
    CommandManager m_manager;
};

#endif // SCENE_PRESENTER_H
