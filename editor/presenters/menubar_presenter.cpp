#include "menubar_presenter.hpp"
#include "../../design_models/include/deserialize.hpp"

MenuBarPresenter::MenuBarPresenter(Context& context, MenuBar& menuBar)
    : m_context(context), m_menuBarView(menuBar)
{
    connect(m_menuBarView.m_fileMenu->m_save, &QAction::triggered, [this]() {
        handleSave();
    });

    connect(m_menuBarView.m_fileMenu->m_saveAs, &QAction::triggered, [this]() {
        handleSaveAs();
    });

    connect(m_menuBarView.m_fileMenu->m_open, &QAction::triggered, [this]() {
        handleOpen();
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

void MenuBarPresenter::handleOpen()
{
    QString name = QFileDialog::getOpenFileName(
        nullptr,
        "Open project",
        "",
        "(*.json)"
    );
    auto stringName = name.toStdString();
    if (stringName.empty())
        return;
    m_currentPath = stringName;
    openFromFile(stringName);
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

void MenuBarPresenter::openFromFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
    {
        qWarning() << "Can not open file for reading:" << QString::fromStdString(path);
        return;
    }
    nlohmann::json j;
    file >> j;
    // Пример: загрузка компонентов
    m_context.m_layout.m_components.clear();
    for (const auto& item : j)
    {
        try {
            auto comp = design_models::deserialize_component(item);
            m_context.m_layout.m_components.push_back(comp);
        } catch (const std::exception& e) {
            qWarning() << "Deserialize error:" << e.what();
        }
    }
    qInfo() << "Loaded" << m_context.m_layout.m_components.size() << "components from file.";
}
