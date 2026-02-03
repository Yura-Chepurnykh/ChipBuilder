#ifndef SCENE_CONTROLLER_HPP
#define SCENE_CONTROLLER_HPP

#include "scene.hpp"
#include "scene_view.hpp"

class SceneController
{
public:
    SceneController(Scene* scene, SceneView* view);

public slots:
    void onSceneChanged();

private:
    Scene* m_sceneView;
    SceneView* m_modelView;
};

#endif // SCENE_CONTROLLER_HPP
