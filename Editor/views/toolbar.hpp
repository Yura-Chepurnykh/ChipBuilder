#ifndef TOOLBAR_HPP
#define TOOLBAR_HPP

#include <QToolBar>
#include <QAction>
#include "scene_view.hpp"
// #include "mainwindow.h"

// class MainWindow;

class SceneView;

class ToolBar : public QToolBar
{
public:
    ToolBar();

    // friend class MainWindow;

// private:
    SceneView* m_scene;

    QAction* m_nSubstrate;
    QAction* m_pSubstrate;
    QAction* m_nSource;
    QAction* m_pSource;
    QAction* m_nDrain;
    QAction* m_pDrain;
    QAction* m_oxide;
    QAction* m_polysilicon;
};

#endif // TOOLBAR_HPP
