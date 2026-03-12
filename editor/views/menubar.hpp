#ifndef MENUBAR_HPP
#define MENUBAR_HPP

#include <QMenuBar>
#include <QMenu>
#include <QAction>

class FileMenu final : public QMenu
{
public:
    FileMenu(QWidget* parent = nullptr) : QMenu("File", parent)
    {
        m_save = new QAction("Save", this);
        m_saveAs = new QAction("Save As", this);
        m_open = new QAction("Open");
        addAction(m_save);
        addAction(m_saveAs);
        addSeparator();
    }

// private:
    QAction* m_save;
    QAction* m_saveAs;
    QAction* m_open;
};

class EditMenu final : public QMenu
{
public:
    EditMenu(QWidget* parent = nullptr) : QMenu("Edit", parent)
    {
        m_undo = new QAction("Undo", this);
        m_redo = new QAction("Redo", this);
        m_zoomIn = new QAction("Zoom In", this);
        m_zoomOut = new QAction("Zoom Out", this);
        addAction(m_undo);
        addAction(m_redo);
        addSeparator();
        addAction(m_zoomIn);
        addAction(m_zoomOut);
    }

    QAction* m_undo;
    QAction* m_redo;
    QAction* m_zoomIn;
    QAction* m_zoomOut;
};

class MenuBar final : public QMenuBar
{
public:
    MenuBar(QWidget* parent = nullptr) : QMenuBar(parent)
    {
        m_fileMenu = new FileMenu(this);
        m_editMenu = new EditMenu(this);
        addMenu(m_fileMenu);
        addMenu(m_editMenu);
    }
    
    FileMenu* m_fileMenu;
    EditMenu* m_editMenu;
};

#endif // MENUBAR_HPP
