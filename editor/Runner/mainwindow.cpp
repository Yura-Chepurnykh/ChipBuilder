#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_context = new Context();

    m_sceneView = new SceneView();
    m_toolbar = new Toolbar();

    m_view = new View(m_sceneView);
    m_view->setScene(m_sceneView);

    m_scenePresenter = new ScenePresenter(*m_context, *m_sceneView);
    m_toolbarPresenter = new ToolbarPresenter(m_toolbar);

    connect(m_toolbarPresenter, &ToolbarPresenter::selectedLayer, m_scenePresenter, &ScenePresenter::onSelectedLayer);

    this->addToolBar(m_toolbar);
    this->setCentralWidget(m_view);
}

MainWindow::~MainWindow()
{
    delete ui;
}
