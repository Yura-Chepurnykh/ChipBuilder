#ifndef TOOLBAR_PRESENTER_H
#define TOOLBAR_PRESENTER_H

#include <memory>
#include <QDebug>
#include <QObject>
#include <typeindex>
#include "toolbar.hpp"
#include "layers.hpp"
#include "id_generator.hpp"
#include "scene_presenter.hpp"

class SceneController;

class ToolbarPresenter : public QObject
{
    Q_OBJECT

public:
    ToolbarPresenter(Toolbar*);

    SceneController* sceneController;

signals:
    void selectedLayer(std::shared_ptr<Layer>);

public slots:
    void handleNSubstrateClick();
    void handlePSubstrateClick();
    void handleNDiffusionClick();
    void handlePDiffusionClick();
    void handleOxideClick();
    void handlePolysiliconClick();

private:
    std::shared_ptr<Layer> m_selectedLayer;
    Toolbar* m_toolbarView;
};

#endif // TOOLBAR_PRESENTER_H
