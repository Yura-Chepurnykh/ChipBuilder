#ifndef TOOLBAR_PRESENTER_HPP
#define TOOLBAR_PRESENTER_HPP

#include <QObject>
#include <memory>
#include "toolbar.hpp"
#include "layers.hpp"
#include "id_generator.hpp"

class ToolbarPresenter : public QObject
{
    Q_OBJECT
public:
    ToolbarPresenter(Toolbar* view);

signals:
    void selectedLayer(std::shared_ptr<Layer> layer);

private slots:
    void handleActiveClick();
    void handlePolyClick();
    void handleNWellClick();
    void handlePWellClick();
    void handleMetal1Click();
    void handleViaClick();
    void handleContactClick();

private:
    Toolbar* m_toolbarView;
    std::shared_ptr<Layer> m_selectedLayer;
};

#endif // TOOLBAR_PRESENTER_HPP
