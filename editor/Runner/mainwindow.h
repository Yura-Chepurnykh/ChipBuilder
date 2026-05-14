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
#include "menubar.hpp"
#include "menubar_presenter.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

#include <QListWidget>

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
    MenuBarPresenter* m_menuBarPresenter;
    MenuBar* m_menuBar;
    View* m_view;
    QListWidget* m_drcSidebar;
};
#endif // MAINWINDOW_H
