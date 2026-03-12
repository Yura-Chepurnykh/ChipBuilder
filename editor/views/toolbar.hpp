#ifndef TOOLBAR_HPP
#define TOOLBAR_HPP

#include <QToolBar>
#include <QAction>
#include <QDebug>
#include <QString>
#include "scene_view.hpp"

class Toolbar : public QToolBar
{
public:
    Toolbar();

    QAction* createAction(const QString& title, const QString& iconPath = QString());

    QAction* m_nSubstrate;
    QAction* m_pSubstrate;
    QAction* m_nDiffusion;
    QAction* m_pDiffusion;
    QAction* m_oxide;
    QAction* m_polysilicon;
};

#endif // TOOLBAR_HPP
