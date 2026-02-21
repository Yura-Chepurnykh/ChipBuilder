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
    ScenePresenter(Context&, SceneView&) noexcept;
    void setState(std::unique_ptr<IStrategy>);
    void handle(const QPointF&);

signals:
    void drawRectPreview();

public slots:
    void handleLayerPress(int);
    void handleSceneClick(const QPointF&);
    void handleMouseMove(const QPointF&);
    void handleMouseRelease(const QPointF&);
    void onSelectedLayer(std::shared_ptr<Layer>);

public slots:
    void handleMKeyPress();
    void handleDeleteKeyPress();

public:
    Context& m_context;
    SceneView& m_view;
    std::unique_ptr<IStrategy> m_strategy;
    std::shared_ptr<Layer> m_selectedLayer;
    std::unique_ptr<IShapeBuilder> m_builder;
    CommandManager m_manager;
};

#endif // SCENE_PRESENTER_H
