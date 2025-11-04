#include "controller/App.h"

#include "common/Protocol.h"
#include "controller/UiMainWindow.h"

#include <QSettings>

namespace controller {

App::App(int &argc, char **argv)
    : QObject(nullptr)
    , m_app(argc, argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("Controller"));
    QCoreApplication::setOrganizationName(QStringLiteral("RemoteDesk"));
}

App::~App() = default;

int App::run()
{
    m_mainWindow = std::make_unique<UiMainWindow>();
    m_mainWindow->setApiBase(QString::fromUtf8(Protocol::kApiBase));
    m_mainWindow->show();

    return m_app.exec();
}

} // namespace controller

int main(int argc, char **argv)
{
    controller::App app(argc, argv);
    return app.run();
}
