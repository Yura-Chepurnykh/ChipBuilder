#include "menubar_presenter.hpp"

MenuBarPresenter::MenuBarPresenter(Context& context, MenuBar& menuBar) : m_context(context), m_menuBarView(menuBar)
{
    connect(m_menuBarView.m_fileMenu->m_save, &QAction::triggered, [this]() {
        handleSave();
    });

    connect(m_menuBarView.m_fileMenu->m_saveAs, &QAction::triggered, [this]() {
        handleSaveAs();
    });
}

void MenuBarPresenter::handleSave()
{
    if (m_currentPath.empty())
    {
        handleSaveAs();
    }

    saveToFile(m_currentPath);
}

void MenuBarPresenter::handleSaveAs()
{
    QString name = QFileDialog::getSaveFileName(
        nullptr,
        "Save project",
        "",
        "(*.json)"
        );

    auto stringName = name.toStdString();

    if (stringName.empty())
        return;

    m_currentPath = stringName;
    saveToFile(stringName);
}

void MenuBarPresenter::saveToFile(const std::string& currentPath)
{
    std::ofstream file(currentPath);

    if (!file)
        throw std::runtime_error("Can not open file");

    JSONSerializer jsonSerializer;

    for (const auto& component : m_context.m_layout.m_components)
    {
        if (component)
        {
            qInfo() << QString::fromStdString(component->name());
            component->accept(jsonSerializer);
        }
    }

    auto json = jsonSerializer.getJson();
    qDebug() << json.dump(4);

    file << json.dump(4);
}
