#include "toolbar.hpp"

Toolbar::Toolbar()
{
    m_active = createAction("Active");
    m_poly = createAction("Poly");
    m_nwell = createAction("NWell");
    m_pwell = createAction("PWell");
    m_metal1 = createAction("Metal1");
    m_via = createAction("Via");
    m_contact = createAction("Contact");
}

// pattern fabric method
QAction* Toolbar::createAction(const QString& title)
{
    QAction* action = new QAction(title, this);
    this->addAction(action);
    return action;
}
