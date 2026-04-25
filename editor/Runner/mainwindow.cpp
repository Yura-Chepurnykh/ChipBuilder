#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_menuBar = new MenuBar(this);
    m_context = new Context();
    m_sceneView = new SceneView();

    this->setMenuBar(m_menuBar);

    m_sceneView->setSceneRect(-5000, -5000, 10000, 10000);

    m_menuBarPresenter = new MenuBarPresenter(*m_context, *m_menuBar, *m_sceneView);

    m_toolbar = new Toolbar();

    // Modern Dark Theme Stylesheet
    QString mainStyle = R"(
        QMainWindow {
            background-color: #1e1e1e;
        }
        QMenuBar {
            background-color: #323233;
            color: #cccccc;
            padding: 2px;
        }
        QMenuBar::item {
            background-color: transparent;
            padding: 4px 10px;
        }
        QMenuBar::item:selected {
            background-color: #454545;
            border-radius: 3px;
        }
        QMenu {
            background-color: #252526;
            color: #cccccc;
            border: 1px solid #454545;
        }
        QMenu::item {
            padding: 6px 25px 6px 20px;
        }
        QMenu::item:selected {
            background-color: #094771;
            color: #ffffff;
        }
        QToolBar {
            background-color: #333333;
            border: none;
            spacing: 5px;
            padding: 5px;
            width: 120px;
        }
        QToolButton {
            background-color: #3c3c3c;
            color: #ffffff;
            border-radius: 4px;
            padding: 8px;
            width: 100px;
            font-size: 11px;
            font-weight: bold;
            text-align: center;
        }
        QToolButton:hover {
            background-color: #505050;
        }
        QToolButton:pressed {
            background-color: #094771;
        }
    )";

    this->setStyleSheet(mainStyle);

    m_view = new View(m_sceneView);
    m_view->setScene(m_sceneView);

    // DRC Violations Panel
    m_drcList = new QListWidget();
    m_drcList->setStyleSheet(R"(
        QListWidget {
            background-color: #252526;
            color: #f1f1f1;
            border: 1px solid #454545;
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 11px;
        }
        QListWidget::item {
            padding: 5px;
            border-bottom: 1px solid #333333;
        }
        QListWidget::item:selected {
            background-color: #094771;
        }
    )");

    m_drcStatusLabel = new QLabel("DRC Status: CLEAN");
    m_drcStatusLabel->setStyleSheet("color: #4ec9b0; font-weight: bold; padding: 5px; background-color: #333333;");

    QVBoxLayout* drcLayout = new QVBoxLayout();
    drcLayout->addWidget(new QLabel("DRC VIOLATIONS:"));
    drcLayout->addWidget(m_drcList);
    drcLayout->addWidget(m_drcStatusLabel);
    
    QWidget* drcContainer = new QWidget();
    drcContainer->setLayout(drcLayout);
    drcContainer->setFixedWidth(250);

    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->addWidget(m_view, 1);
    mainLayout->addWidget(drcContainer);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    QWidget* centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);

    m_scenePresenter = new ScenePresenter(*m_context, *m_sceneView);
    m_toolbarPresenter = new ToolbarPresenter(m_toolbar);

    // Connect violations list
    connect(m_scenePresenter, &ScenePresenter::drcViolationsFound, this, [this](QStringList messages) {
        m_drcList->clear();
        if (messages.isEmpty()) {
            m_drcStatusLabel->setText("DRC Status: CLEAN");
            m_drcStatusLabel->setStyleSheet("color: #4ec9b0; font-weight: bold; padding: 5px; background-color: #333333;");
        } else {
            m_drcList->addItems(messages);
            m_drcStatusLabel->setText(QString("DRC Status: %1 ERRORS").arg(messages.size()));
            m_drcStatusLabel->setStyleSheet("color: #f44336; font-weight: bold; padding: 5px; background-color: #333333;");
        }
    });

    connect(m_menuBarPresenter, &MenuBarPresenter::rectSelectionTriggered, m_scenePresenter, &ScenePresenter::handleRectSelectionTriggered);
    connect(m_menuBarPresenter, &MenuBarPresenter::lassoSelectionTriggered, m_scenePresenter, &ScenePresenter::handleLassoSelectionTriggered);
    connect(m_menuBarPresenter, &MenuBarPresenter::panningTriggered, m_scenePresenter, &ScenePresenter::handlePanningTriggered);
    connect(m_menuBarPresenter, &MenuBarPresenter::loadRulesTriggered, m_scenePresenter, &ScenePresenter::loadRules);

    connect(m_menuBarPresenter, &MenuBarPresenter::undoTriggered, m_scenePresenter, &ScenePresenter::handleUndoPress);
    connect(m_menuBarPresenter, &MenuBarPresenter::redoTriggered, m_scenePresenter, &ScenePresenter::handleRedoPress);
    connect(m_menuBarPresenter, &MenuBarPresenter::zoomInTriggered, m_scenePresenter, &ScenePresenter::handleZoomIn);
    connect(m_menuBarPresenter, &MenuBarPresenter::zoomOutTriggered, m_scenePresenter, &ScenePresenter::handleZoomOut);
    connect(m_menuBarPresenter, &MenuBarPresenter::groupTriggered, m_scenePresenter, &ScenePresenter::handleGroupUngroup);
    connect(m_menuBarPresenter, &MenuBarPresenter::viewCreated, m_scenePresenter, &ScenePresenter::bindView);

    connect(m_toolbarPresenter, &ToolbarPresenter::selectedLayer, m_scenePresenter, &ScenePresenter::onSelectedLayer);

    // Try to load default rules from various possible locations
    QStringList possiblePaths = {
        "../../rules.json",
        "../rules.json",
        "./rules.json",
        "rules.json"
    };
    
    for (const auto& path : possiblePaths) {
        if (QFile::exists(path)) {
            m_scenePresenter->loadRules(path);
            break;
        }
    }

    this->addToolBar(Qt::LeftToolBarArea, m_toolbar);
    this->setCentralWidget(centralWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}
