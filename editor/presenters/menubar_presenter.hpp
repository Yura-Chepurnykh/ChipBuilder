#ifndef MENUBAR_PRESENTER_HPP
#define MENUBAR_PRESENTER_HPP

#include <QObject>
#include <QFileDialog>
#include <QString>
#include <QDebug>
#include <QGraphicsItem>
#include <fstream>
#include "menubar.hpp"
#include "context.hpp"

#include "layers.hpp"

class MenuBarPresenter final : public QObject
{
    Q_OBJECT
public:
    MenuBarPresenter(Context& context, MenuBar& menuBar, class SceneView& sceneView);

public slots:
    void handleNew();
    void handleOpen();
    void handleSave();
    void handleSaveAs();
    void handleLoadDRC();

signals:
    void rectSelectionTriggered();
    void lassoSelectionTriggered();
    void panningTriggered();

    void undoTriggered();
    void redoTriggered();
    void zoomInTriggered();
    void zoomOutTriggered();
    void groupTriggered();

    void viewCreated(QGraphicsItem*);

private:
    void saveToFile(const std::string& currentPath);
    void loadFromFile(const std::string& currentPath);

private:
    std::string m_currentPath;
    Context& m_context;
    MenuBar& m_menuBarView;
    class SceneView& m_sceneView;
};

#endif // MENUBAR_PRESENTER_HPP
