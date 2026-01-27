#include "toolbar.hpp"

ToolBar::ToolBar()
{
    m_nSubstrate = new QAction("N-Substrate", this);
    m_pSubstrate = new QAction("P-Substrate", this);
    m_nSource = new QAction("N-Source", this);
    m_pSource = new QAction("N-Source", this);
    m_nDrain = new QAction("N-Drain", this);
    m_pDrain = new QAction("N-Drain", this);
    m_oxide = new QAction("Oxide", this);
    m_polysilicon = new QAction("N-Polysilicon", this);

    this->addAction(m_nSubstrate);
    this->addAction(m_pSubstrate);
    this->addAction(m_nSource);
    this->addAction(m_pSource);
    this->addAction(m_nDrain);
    this->addAction(m_pDrain);
    this->addAction(m_oxide);
    this->addAction(m_polysilicon);
}
