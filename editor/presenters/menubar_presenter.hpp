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
    MenuBarPresenter(Context& context, MenuBar& menuBar) : m_context(context), m_menuBarView(menuBar)
    {
        connect(m_menuBarView.m_fileMenu->m_save, &QAction::triggered, [this]() {
            handleSave();
        });

        connect(m_menuBarView.m_fileMenu->m_saveAs, &QAction::triggered, [this]() {
            handleSaveAs();
        });
    }

public slots:
    void handleSave()
    {
        if (m_currentPath.empty())
        {
            handleSaveAs();
        }

        saveToFile(m_currentPath);
    }

    void handleSaveAs()
    {
        QString name = QFileDialog::getSaveFileName(
            nullptr,
            "Save project",
            "",
            "(*.json)"
        );

        auto stringName = name.toStdString();

        if (stringName.empty())
            return;

        m_currentPath = stringName;
        saveToFile(stringName);
    }

private:
    void saveToFile(const std::string& currentPath)
    {
        std::ofstream file(currentPath);

        if (!file)
            throw std::runtime_error("Can not open file");

        JSONSerializer jsonSerializer;

        // m_context.m_layout.accept(jsonSerializer);

        for (const auto& c : m_context.m_layout.m_components)
        {
            if (c)
            {
                qInfo() << QString::fromStdString(c->name());
                c->accept(jsonSerializer);
            }
        }

        auto json = jsonSerializer.getJson();
        qDebug() << json.dump(4);

        file << json.dump(4);
    }

private:
    std::string m_currentPath;
    Context& m_context;
    MenuBar& m_menuBarView;
};

#endif // MENUBAR_PRESENTER_HPP
