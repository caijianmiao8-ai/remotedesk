#pragma once

#include <memory>

#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QStatusBar>
#include <QVBoxLayout>

namespace controller {

class UiMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit UiMainWindow(QWidget *parent = nullptr);
    ~UiMainWindow() override;

    void setApiBase(const QString &baseUrl);
    void setUserCode(const QString &userCode, int expiresInSeconds);
    void setSessionCode(const QString &code6);
    void setConnectionStatus(const QString &statusText);
    void setMetricsText(const QString &metrics);
    void showVideoFrame(const QImage &frame);

signals:
    void requestLogin();
    void requestCreateSession();
    void requestJoinSession(const QString &code6);
    void requestConnect();
    void requestDisconnect();

private slots:
    void onJoinButtonClicked();

private:
    void buildUi();

    QString m_apiBase;
    QWidget *m_centralWidget = nullptr;
    QLabel *m_loginInfoLabel = nullptr;
    QLabel *m_sessionCodeLabel = nullptr;
    QPushButton *m_loginButton = nullptr;
    QPushButton *m_createSessionButton = nullptr;
    QPushButton *m_connectButton = nullptr;
    QPushButton *m_disconnectButton = nullptr;
    QLineEdit *m_joinCodeEdit = nullptr;
    QLabel *m_videoLabel = nullptr;
    QLabel *m_metricsLabel = nullptr;
};

} // namespace controller
