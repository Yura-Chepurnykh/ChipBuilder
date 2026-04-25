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

    QAction* createAction(const QString&);

    QAction* m_active;
    QAction* m_poly;
    QAction* m_nwell;
    QAction* m_pwell;
    QAction* m_metal1;
    QAction* m_via;
    QAction* m_contact;
};

#endif // TOOLBAR_HPP
