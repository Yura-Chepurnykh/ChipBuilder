#include "toolbar_presenter.hpp"

ToolbarPresenter::ToolbarPresenter(Toolbar* view) : m_toolbarView(view), m_selectedLayer(nullptr)
{
    connect(m_toolbarView->m_active,  &QAction::triggered, this, &ToolbarPresenter::handleActiveClick);
    connect(m_toolbarView->m_poly,    &QAction::triggered, this, &ToolbarPresenter::handlePolyClick);
    connect(m_toolbarView->m_nwell,   &QAction::triggered, this, &ToolbarPresenter::handleNWellClick);
    connect(m_toolbarView->m_pwell,   &QAction::triggered, this, &ToolbarPresenter::handlePWellClick);
    connect(m_toolbarView->m_metal1,  &QAction::triggered, this, &ToolbarPresenter::handleMetal1Click);
    connect(m_toolbarView->m_via,     &QAction::triggered, this, &ToolbarPresenter::handleViaClick);
    connect(m_toolbarView->m_contact, &QAction::triggered, this, &ToolbarPresenter::handleContactClick);
}

void ToolbarPresenter::handleActiveClick()
{
    m_selectedLayer = std::make_shared<Active>(IdGenerator::generate(), 0, nullptr);
    emit selectedLayer(m_selectedLayer);
}

void ToolbarPresenter::handlePolyClick()
{
    m_selectedLayer = std::make_shared<Poly>(IdGenerator::generate(), 0, nullptr);
    emit selectedLayer(m_selectedLayer);
}

void ToolbarPresenter::handleNWellClick()
{
    m_selectedLayer = std::make_shared<NWell>(IdGenerator::generate(), 0, nullptr);
    emit selectedLayer(m_selectedLayer);
}

void ToolbarPresenter::handlePWellClick()
{
    m_selectedLayer = std::make_shared<PWell>(IdGenerator::generate(), 0, nullptr);
    emit selectedLayer(m_selectedLayer);
}

void ToolbarPresenter::handleMetal1Click()
{
    m_selectedLayer = std::make_shared<Metal1>(IdGenerator::generate(), 0, nullptr);
    emit selectedLayer(m_selectedLayer);
}

void ToolbarPresenter::handleViaClick()
{
    m_selectedLayer = std::make_shared<Via>(IdGenerator::generate(), 0, nullptr);
    emit selectedLayer(m_selectedLayer);
}

void ToolbarPresenter::handleContactClick()
{
    m_selectedLayer = std::make_shared<Contact>(IdGenerator::generate(), 0, nullptr);
    emit selectedLayer(m_selectedLayer);
}
