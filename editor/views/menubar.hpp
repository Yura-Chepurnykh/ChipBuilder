#ifndef MENUBAR_HPP
#define MENUBAR_HPP

#include <QMenuBar>
#include <QMenu>
#include <QAction>

class FileMenu final : public QMenu
{
public:
    FileMenu(QWidget* parent = nullptr) : QMenu("file", parent)
    {
        m_save = new QAction("save", this);
        m_saveAs = new QAction("save as", this);

        addAction(m_save);
        addAction(m_saveAs);
    }

// private:
    QAction* m_save;
    QAction* m_saveAs;
};

class MenuBar final : public QMenuBar
{
public:
    MenuBar(QWidget* parent = nullptr) : QMenuBar(parent)
    {
        m_fileMenu = new FileMenu(this);
        addMenu(m_fileMenu);
    }

// private:
    FileMenu* m_fileMenu;
};

#endif // MENUBAR_HPP
