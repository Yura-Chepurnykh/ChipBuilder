#ifndef SCENE_MODEL_HPP
#define SCENE_MODEL_HPP

class SceneModel
{
public:
    SceneModel() noexcept : m_isMetalActivated(false), m_showGrid(false) { }

    void setMetalActivated(bool isActivated) noexcept { m_isMetalActivated = isActivated; }
    bool isMetalActivated() const noexcept { return m_isMetalActivated; }

    void setShowGrid(bool isActivated) noexcept { m_showGrid = isActivated; }
    bool isShowGrid() const noexcept { return m_showGrid; }

private:
    bool m_isMetalActivated, m_showGrid;
};

#endif // SCENE_MODEL_HPP
