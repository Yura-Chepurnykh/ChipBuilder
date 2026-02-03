#include "scene_controller.hpp"

SceneController::SceneController(Scene* scene, SceneView* view) : m_sceneView(scene), m_modelView(view)
{
    connect(m_sceneView, &Scene::changed, this, &SceneController::onSceneChanged);
}

void SceneController::onSceneChanged()
{
    m_modelView->update();
}
