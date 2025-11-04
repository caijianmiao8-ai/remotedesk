#include "controller/AuthClient.h"

#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace controller {

namespace {
QNetworkRequest makeJsonRequest(const QUrl &url)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    return request;
}

} // namespace

AuthClient::AuthClient(QObject *parent)
    : QObject(parent)
{
}

void AuthClient::setApiBase(const QUrl &baseUrl)
{
    m_apiBase = baseUrl;
}

QUrl AuthClient::apiBase() const
{
    return m_apiBase;
}

void AuthClient::setAppToken(const QString &token)
{
    m_appToken = token;
}

QString AuthClient::appToken() const
{
    return m_appToken;
}

void AuthClient::startDeviceFlow()
{
    if (!m_apiBase.isValid()) {
        emit requestFailed(QStringLiteral("device/start"), QStringLiteral("API base URL is not set"));
        return;
    }

    const QUrl url = m_apiBase.resolved(QUrl(QStringLiteral("/api/device/start")));
    auto reply = m_network.post(makeJsonRequest(url), QByteArrayLiteral("{}"));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            handleNetworkError(QStringLiteral("device/start"), reply);
            return;
        }

        const auto json = QJsonDocument::fromJson(reply->readAll()).object();
        DeviceStartResponse response;
        response.deviceCode = json.value(QStringLiteral("device_code")).toString();
        response.userCode = json.value(QStringLiteral("user_code")).toString();
        response.verificationUri = json.value(QStringLiteral("verification_uri")).toString();
        response.intervalSeconds = json.value(QStringLiteral("interval")).toInt(5);
        response.expiresInSeconds = json.value(QStringLiteral("expires_in")).toInt(600);
        emit deviceFlowStarted(response);
    });
}

void AuthClient::pollDeviceCode(const QString &deviceCode)
{
    if (!m_apiBase.isValid()) {
        emit requestFailed(QStringLiteral("device/poll"), QStringLiteral("API base URL is not set"));
        return;
    }

    const QUrl url = m_apiBase.resolved(QUrl(QStringLiteral("/api/device/poll")));
    const QJsonObject payload{
        {QStringLiteral("device_code"), deviceCode},
    };
    auto reply = m_network.post(makeJsonRequest(url), QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            handleNetworkError(QStringLiteral("device/poll"), reply);
            return;
        }

        const auto json = QJsonDocument::fromJson(reply->readAll()).object();
        const auto status = json.value(QStringLiteral("status")).toString();
        if (status == QStringLiteral("approved")) {
            DevicePollApproved approved;
            approved.appToken = json.value(QStringLiteral("app_token")).toString();
            const auto user = json.value(QStringLiteral("user")).toObject();
            approved.userId = user.value(QStringLiteral("id")).toString();
            m_appToken = approved.appToken;
            emit deviceFlowApproved(approved);
        } else {
            emit deviceFlowPending();
        }
    });
}

void AuthClient::handleNetworkError(const QString &context, QNetworkReply *reply)
{
    const auto message = reply->errorString();
    emit requestFailed(context, message);
}

} // namespace controller
