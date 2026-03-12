#ifndef MENUBAR_PRESENTER_HPP
#define MENUBAR_PRESENTER_HPP

#include <QObject>
#include <QFileDialog>
#include <QString>
#include <QDebug>
#include <fstream>
#include "menubar.hpp"
#include "context.hpp"

class MenuBarPresenter final : public QObject
{
public:
    MenuBarPresenter(Context& context, MenuBar& menuBar);

public slots:
    void handleSave();
    void handleSaveAs();
    void handleOpen();

private:
    void saveToFile(const std::string& currentPath);
    void openFromFile(const std::string& path);

private:
    std::string m_currentPath;
    Context& m_context;
    MenuBar& m_menuBarView;
};

#endif // MENUBAR_PRESENTER_HPP
