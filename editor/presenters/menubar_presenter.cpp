#include "menubar_presenter.hpp"
#include "scene_view.hpp"
#include "factories.hpp"
#include "layer_model.hpp"

MenuBarPresenter::MenuBarPresenter(Context& context, MenuBar& menuBar, SceneView& sceneView) : 
    m_context(context), m_menuBarView(menuBar), m_sceneView(sceneView)
{
    connect(m_menuBarView.m_fileMenu->m_new, &QAction::triggered, [this]() {
        handleNew();
    });

    connect(m_menuBarView.m_fileMenu->m_open, &QAction::triggered, [this]() {
        handleOpen();
    });

    connect(m_menuBarView.m_fileMenu->m_save, &QAction::triggered, [this]() {
        handleSave();
    });

    connect(m_menuBarView.m_fileMenu->m_saveAs, &QAction::triggered, [this]() {
        handleSaveAs();
    });

    connect(m_menuBarView.m_selectionMenu->m_rectSelection, &QAction::triggered, [this]() {
        emit rectSelectionTriggered();
    });

    connect(m_menuBarView.m_selectionMenu->m_lassoSelection, &QAction::triggered, [this]() {
        emit lassoSelectionTriggered();
    });

    connect(m_menuBarView.m_selectionMenu->m_panning, &QAction::triggered, [this]() {
        emit panningTriggered();
    });

    connect(m_menuBarView.m_editMenu->m_undo, &QAction::triggered, [this]() {
        emit undoTriggered();
    });

    connect(m_menuBarView.m_editMenu->m_redo, &QAction::triggered, [this]() {
        emit redoTriggered();
    });

    connect(m_menuBarView.m_editMenu->m_zoomIn, &QAction::triggered, [this]() {
        emit zoomInTriggered();
    });

    connect(m_menuBarView.m_editMenu->m_zoomOut, &QAction::triggered, [this]() {
        emit zoomOutTriggered();
    });

    connect(m_menuBarView.m_editMenu->m_group, &QAction::triggered, [this]() {
        emit groupTriggered();
    });
}

void MenuBarPresenter::handleNew()
{
    m_currentPath.clear();
    m_context.m_layout.m_components.clear();
    m_context.m_modelToView.clear();
    m_context.m_viewToModel.clear();
    m_sceneView.clear();
    m_sceneView.update();
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
    loadFromFile(stringName);
}

void MenuBarPresenter::loadFromFile(const std::string& currentPath)
{
    std::ifstream file(currentPath);

    if (!file)
    {
        qWarning() << "Can not open file for reading: " << QString::fromStdString(currentPath);
        return;
    }

    JSON json;
    file >> json;

    auto components = JSONDeserializer::deserialize(json);
    
    // Clear current scene and layout
    m_context.m_layout.m_components.clear();
    m_context.m_modelToView.clear();
    m_context.m_viewToModel.clear();
    m_sceneView.clear();

    for (auto& component : components)
    {
        if (component)
        {
            m_context.m_layout.add(component);
            
            auto shape = component->getShape();
            if (shape)
            {
                auto styleOpt = StyleModel().getStyle(typeid(*component));
                if (styleOpt)
                {
                    auto viewItem = ViewFactory::create(shape, *styleOpt, QString::fromStdString(component->name()));
                    m_sceneView.addItem(viewItem);
                    emit viewCreated(viewItem);

                    // Add mapping between model and view
                    if (auto layerView = dynamic_cast<LayerView*>(viewItem))
                    {
                        m_context.m_modelToView[component->id] = layerView->id;
                        m_context.m_viewToModel[layerView->id] = component->id;
                    }
                    else if (auto metalView = dynamic_cast<MetalView*>(viewItem))
                    {
                        m_context.m_modelToView[component->id] = metalView->id;
                        m_context.m_viewToModel[metalView->id] = component->id;
                    }
                }
            }
        }
    }
    m_sceneView.update();
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
