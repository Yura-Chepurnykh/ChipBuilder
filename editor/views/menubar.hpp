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
        m_new = new QAction("New", this);
        m_open = new QAction("Open", this);
        m_save = new QAction("Save", this);
        m_saveAs = new QAction("Save As", this);

        addAction(m_new);
        addAction(m_open);
        addAction(m_save);
        addAction(m_saveAs);
    }

// private:
    QAction* m_new;
    QAction* m_open;
    QAction* m_save;
    QAction* m_saveAs;
};

class SelectionMenu final : public QMenu
{
public:
    SelectionMenu(QWidget* parent = nullptr) : QMenu("Selection", parent)
    {
        m_rectSelection = new QAction("Box", this);
        m_lassoSelection = new QAction("Lasso", this);
        m_panning = new QAction("Panning", this);

        addAction(m_rectSelection);
        addAction(m_lassoSelection);
        addAction(m_panning);
    }

    QAction* m_rectSelection;
    QAction* m_lassoSelection;
    QAction* m_panning;
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
        m_group = new QAction("Group/Ungroup", this);

        addAction(m_undo);
        addAction(m_redo);
        addSeparator();
        addAction(m_zoomIn);
        addAction(m_zoomOut);
        addSeparator();
        addAction(m_group);
    }

    QAction* m_undo;
    QAction* m_redo;
    QAction* m_zoomIn;
    QAction* m_zoomOut;
    QAction* m_group;
};

class MenuBar final : public QMenuBar
{
public:
    MenuBar(QWidget* parent = nullptr) : QMenuBar(parent)
    {
        m_fileMenu = new FileMenu(this);
        m_editMenu = new EditMenu(this);
        m_selectionMenu = new SelectionMenu(this);
        addMenu(m_fileMenu);
        addMenu(m_editMenu);
        addMenu(m_selectionMenu);
    }

// private:
    FileMenu* m_fileMenu;
    EditMenu* m_editMenu;
    SelectionMenu* m_selectionMenu;
};

#endif // MENUBAR_HPP
