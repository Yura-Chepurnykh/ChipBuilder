#include "toolbar.hpp"

Toolbar::Toolbar()
{
    m_nSubstrate = createAction("N-Substrate");
    m_pSubstrate = createAction("P-Substrate");
    m_nDiffusion = createAction("N-Diffusion");
    m_pDiffusion = createAction("P-Diffusion");
    m_oxide = createAction("Oxide");
    m_polysilicon = createAction("N-Polysilicon");
}

// pattern fabric method
QAction* Toolbar::createAction(const QString& title)
{
    QAction* action = new QAction(title, this);
    this->addAction(action);
    return action;
}
