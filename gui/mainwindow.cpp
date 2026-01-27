#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_scene = new Scene();

    // testing layer
    QRectF r(10, 10, 40, 40);
    QColor c(Qt::red);

    Layer* layer = new Layer(r, c);

    m_scene->add(layer);

    m_view = new QGraphicsView(m_scene);
    m_view->setScene(m_scene);

    this->setCentralWidget(m_view);
}

MainWindow::~MainWindow()
{
    delete ui;
}
