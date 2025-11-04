#include "controller/UiMainWindow.h"

#include <QBoxLayout>
#include <QGuiApplication>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QStatusBar>

namespace controller {

UiMainWindow::UiMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    buildUi();
    setWindowTitle(QStringLiteral("RemoteDesk Controller"));
}

UiMainWindow::~UiMainWindow() = default;

void UiMainWindow::buildUi()
{
    m_centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(m_centralWidget);

    m_loginInfoLabel = new QLabel(tr("Click \"Login\" to start device authorization."), m_centralWidget);
    m_loginButton = new QPushButton(tr("Login"), m_centralWidget);
    connect(m_loginButton, &QPushButton::clicked, this, &UiMainWindow::requestLogin);

    m_createSessionButton = new QPushButton(tr("Create Session"), m_centralWidget);
    connect(m_createSessionButton, &QPushButton::clicked, this, &UiMainWindow::requestCreateSession);

    auto *joinLayout = new QHBoxLayout();
    m_joinCodeEdit = new QLineEdit(m_centralWidget);
    m_joinCodeEdit->setPlaceholderText(tr("Enter code6"));
    auto *joinButton = new QPushButton(tr("Join"), m_centralWidget);
    connect(joinButton, &QPushButton::clicked, this, &UiMainWindow::onJoinButtonClicked);
    joinLayout->addWidget(m_joinCodeEdit, 1);
    joinLayout->addWidget(joinButton);

    m_sessionCodeLabel = new QLabel(tr("Session Code: --"), m_centralWidget);

    m_connectButton = new QPushButton(tr("Connect"), m_centralWidget);
    connect(m_connectButton, &QPushButton::clicked, this, &UiMainWindow::requestConnect);

    m_disconnectButton = new QPushButton(tr("Disconnect"), m_centralWidget);
    connect(m_disconnectButton, &QPushButton::clicked, this, &UiMainWindow::requestDisconnect);

    m_videoLabel = new QLabel(m_centralWidget);
    m_videoLabel->setMinimumSize(640, 360);
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setText(tr("Waiting for video"));

    m_metricsLabel = new QLabel(tr("Metrics: --"), m_centralWidget);

    layout->addWidget(m_loginInfoLabel);
    layout->addWidget(m_loginButton);
    layout->addWidget(m_createSessionButton);
    layout->addLayout(joinLayout);
    layout->addWidget(m_sessionCodeLabel);
    layout->addWidget(m_connectButton);
    layout->addWidget(m_disconnectButton);
    layout->addWidget(m_videoLabel, 1);
    layout->addWidget(m_metricsLabel);

    setCentralWidget(m_centralWidget);

    statusBar()->showMessage(tr("Disconnected"));
}

void UiMainWindow::setApiBase(const QString &baseUrl)
{
    m_apiBase = baseUrl;
    statusBar()->showMessage(tr("API: %1").arg(baseUrl));
}

void UiMainWindow::setUserCode(const QString &userCode, int expiresInSeconds)
{
    m_loginInfoLabel->setText(tr("Device code: %1 (expires in %2s). Visit /device to authorize.")
                                  .arg(userCode, QString::number(expiresInSeconds)));
}

void UiMainWindow::setSessionCode(const QString &code6)
{
    m_sessionCodeLabel->setText(tr("Session Code: %1").arg(code6));
}

void UiMainWindow::setConnectionStatus(const QString &statusText)
{
    statusBar()->showMessage(statusText);
}

void UiMainWindow::setMetricsText(const QString &metrics)
{
    m_metricsLabel->setText(tr("Metrics: %1").arg(metrics));
}

void UiMainWindow::showVideoFrame(const QImage &frame)
{
    if (!frame.isNull()) {
        m_videoLabel->setPixmap(QPixmap::fromImage(frame).scaled(m_videoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void UiMainWindow::onJoinButtonClicked()
{
    const auto code = m_joinCodeEdit->text().trimmed();
    if (!code.isEmpty()) {
        emit requestJoinSession(code);
    }
}

} // namespace controller
