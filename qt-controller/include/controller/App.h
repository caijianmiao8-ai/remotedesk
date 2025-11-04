#pragma once

#include <memory>

#include <QApplication>
#include <QObject>

namespace controller {

class UiMainWindow;

class App : public QObject
{
    Q_OBJECT

public:
    App(int &argc, char **argv);
    ~App() override;

    int run();

private:
    QApplication m_app;
    std::unique_ptr<UiMainWindow> m_mainWindow;
};

} // namespace controller
