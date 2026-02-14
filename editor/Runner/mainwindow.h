#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QObject>
#include "scene_view.hpp"
#include "scene_presenter.hpp"
#include "toolbar_presenter.hpp"
#include "view.hpp"
#include "scene_model.hpp"
#include "toolbar.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    ScenePresenter* m_scenePresenter;
    ToolbarPresenter* m_toolbarPresenter;
    SceneView* m_sceneView;
    Toolbar* m_toolbar;
    Context* m_context;
    View* m_view;
};
#endif // MAINWINDOW_H
