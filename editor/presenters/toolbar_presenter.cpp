#include "toolbar_presenter.hpp"

ToolbarPresenter::ToolbarPresenter(Toolbar* view) : m_toolbarView(view), m_selectedLayer(nullptr)
{
    connect(m_toolbarView->m_nSubstrate,  &QAction::triggered, this, &ToolbarPresenter::handleNSubstrateClick);
    connect(m_toolbarView->m_pSubstrate,  &QAction::triggered, this, &ToolbarPresenter::handlePSubstrateClick);
    connect(m_toolbarView->m_nDiffusion,  &QAction::triggered, this, &ToolbarPresenter::handleNDiffusionClick);
    connect(m_toolbarView->m_pDiffusion,  &QAction::triggered, this, &ToolbarPresenter::handlePDiffusionClick);
    connect(m_toolbarView->m_oxide,       &QAction::triggered, this, &ToolbarPresenter::handleOxideClick);
    connect(m_toolbarView->m_polysilicon, &QAction::triggered, this, &ToolbarPresenter::handlePolysiliconClick);
}

void ToolbarPresenter::handleNSubstrateClick()
{
    m_selectedLayer = std::make_unique<Substrate<NType>>(IdGenerator::generate(), nullptr);
    emit selectedLayer(m_selectedLayer.get());
}

void ToolbarPresenter::handlePSubstrateClick()
{
    m_selectedLayer = std::make_unique<Substrate<PType>>(IdGenerator::generate(), nullptr);
    emit selectedLayer(m_selectedLayer.get());
}

void ToolbarPresenter::handleNDiffusionClick()
{
    m_selectedLayer = std::make_unique<Diffusion<NType>>(IdGenerator::generate(), nullptr);
    emit selectedLayer(m_selectedLayer.get());
}

void ToolbarPresenter::handlePDiffusionClick()
{
    m_selectedLayer = std::make_unique<Diffusion<PType>>(IdGenerator::generate(), nullptr);
    emit selectedLayer(m_selectedLayer.get());
}

void ToolbarPresenter::handleOxideClick()
{
    m_selectedLayer = std::make_unique<Oxide>(IdGenerator::generate(), nullptr);
    emit selectedLayer(m_selectedLayer.get());
}

void ToolbarPresenter::handlePolysiliconClick()
{
    m_selectedLayer = std::make_unique<PolySilicon>(IdGenerator::generate(), nullptr);
    emit selectedLayer(m_selectedLayer.get());
}
