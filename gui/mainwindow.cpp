#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_scene = new Scene(50);

    // testing layer
    QRectF r(10, 10, 40, 40);
    // QColor c(Qt::red);

    Layer* layer = new Layer(r, Layer::Type::NSource);

    m_scene->add(layer);

    m_view = new View(m_scene);
    m_view->setScene(m_scene);

    m_toolBar = new ToolBar();

    this->addToolBar(m_toolBar);
    this->setCentralWidget(m_view);

    connect(m_toolBar->m_nSubstrate, &QAction::triggered, m_scene, &Scene::nSubstrate);
    connect(m_toolBar->m_pSubstrate, &QAction::triggered, m_scene, &Scene::pSubstrate);
    connect(m_toolBar->m_nSource, &QAction::triggered, m_scene, &Scene::nSource);
    connect(m_toolBar->m_pSource, &QAction::triggered, m_scene, &Scene::pSource);
    connect(m_toolBar->m_nDrain, &QAction::triggered, m_scene, &Scene::nDrain);
    connect(m_toolBar->m_pDrain, &QAction::triggered, m_scene, &Scene::pDrain);
    connect(m_toolBar->m_oxide, &QAction::triggered, m_scene, &Scene::oxide);
    connect(m_toolBar->m_polysilicon, &QAction::triggered, m_scene, &Scene::polysilicon);
}

MainWindow::~MainWindow()
{
    delete ui;
}
