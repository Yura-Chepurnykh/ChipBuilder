#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_menuBar = new MenuBar(this);
    m_context = new Context();

    this->setMenuBar(m_menuBar);

    m_menuBarPresenter = new MenuBarPresenter(*m_context, *m_menuBar);

    m_sceneView = new SceneView();
    m_toolbar = new Toolbar();

    m_toolbar->setStyleSheet(R"(
        background-color: #252526;
        color: #848484;
    )");

    m_view = new View(m_sceneView);
    m_view->setScene(m_sceneView);

    m_scenePresenter = new ScenePresenter(*m_context, *m_sceneView);
    m_toolbarPresenter = new ToolbarPresenter(m_toolbar);

    connect(m_toolbarPresenter, &ToolbarPresenter::selectedLayer, m_scenePresenter, &ScenePresenter::onSelectedLayer);

    this->addToolBar(Qt::LeftToolBarArea, m_toolbar);
    this->setCentralWidget(m_view);
}

MainWindow::~MainWindow()
{
    delete ui;
}
