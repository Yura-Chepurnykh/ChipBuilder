#include "toolbar.hpp"

QAction* Toolbar::createAction(const QString& title, const QString& iconPath)
{
    QAction* action;
    if (!iconPath.isEmpty()) {
        action = new QAction(QIcon(iconPath), title, this);
    } else {
        action = new QAction(title, this);
    }
    this->addAction(action);
    return action;
}

Toolbar::Toolbar()
{
    m_nSubstrate = createAction("N-Substrate", ":/icons/n_substrate.png");
    m_pSubstrate = createAction("P-Substrate", ":/icons/p_substrate.png");
    m_nDiffusion = createAction("N-Diffusion", ":/icons/n_diffusion.png");
    m_pDiffusion = createAction("P-Diffusion", ":/icons/p_diffusion.png");
    m_oxide = createAction("Oxide", ":/icons/oxide.png");
    m_polysilicon = createAction("N-Polysilicon", ":/icons/polysilicon.png");
}
