#include "mainwindow.h"
#include "ui_mainwindow.h"

// #include "metal.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_scene = new Scene(50);
    m_view = new View(m_scene);
    m_view->setScene(m_scene);
    m_toolBar = new ToolBar();

    this->addToolBar(m_toolBar);
    this->setCentralWidget(m_view);

    // creation of layers
    connect(m_toolBar->m_nSubstrate, &QAction::triggered, m_scene, &Scene::nSubstrateCreate);
    connect(m_toolBar->m_pSubstrate, &QAction::triggered, m_scene, &Scene::pSubstrateCreate);
    connect(m_toolBar->m_nSource, &QAction::triggered, m_scene, &Scene::nSourceCreate);
    connect(m_toolBar->m_pSource, &QAction::triggered, m_scene, &Scene::pSourceCreate);
    connect(m_toolBar->m_nDrain, &QAction::triggered, m_scene, &Scene::nDrainCreate);
    connect(m_toolBar->m_pDrain, &QAction::triggered, m_scene, &Scene::pDrainCreate);
    connect(m_toolBar->m_oxide, &QAction::triggered, m_scene, &Scene::oxideCreate);
    connect(m_toolBar->m_polysilicon, &QAction::triggered, m_scene, &Scene::polysiliconCreate);

    Metal* metal = new Metal;


    m_scene->addItem(metal);

    metal->add(std::make_shared<QPointF>(10, 10));
    metal->add(std::make_shared<QPointF>(20, 10));

    metal->add(std::make_shared<QPointF>(30, 30));
    metal->add(std::make_shared<QPointF>(15, 40));
}

MainWindow::~MainWindow()
{
    delete ui;
}
