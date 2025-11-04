#pragma once

#include <functional>

#include <QDateTime>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QUrl>

class QNetworkReply;

namespace controller {

struct DeviceStartResponse
{
    QString deviceCode;
    QString userCode;
    QString verificationUri;
    int intervalSeconds = 5;
    int expiresInSeconds = 600;
};

struct DevicePollApproved
{
    QString appToken;
    QString userId;
};

class AuthClient : public QObject
{
    Q_OBJECT

public:
    explicit AuthClient(QObject *parent = nullptr);

    void setApiBase(const QUrl &baseUrl);
    QUrl apiBase() const;

    void setAppToken(const QString &token);
    QString appToken() const;

public slots:
    void startDeviceFlow();
    void pollDeviceCode(const QString &deviceCode);

signals:
    void deviceFlowStarted(const controller::DeviceStartResponse &response);
    void deviceFlowPending();
    void deviceFlowApproved(const controller::DevicePollApproved &response);
    void requestFailed(const QString &context, const QString &errorString);

private:
    void handleNetworkError(const QString &context, QNetworkReply *reply);

    QUrl m_apiBase;
    QString m_appToken;
    QNetworkAccessManager m_network;
};

} // namespace controller
