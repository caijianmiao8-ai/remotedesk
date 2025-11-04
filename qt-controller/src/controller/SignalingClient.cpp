#include "controller/SignalingClient.h"

#include <QAbstractSocket>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonValue>
#include <QNetworkRequest>

namespace controller {

SignalingClient::SignalingClient(QObject *parent)
    : QObject(parent)
{
    connect(&m_socket, &QWebSocket::connected, this, &SignalingClient::sendJoinMessage);
    connect(&m_socket, &QWebSocket::disconnected, this, [this]() {
        m_joined = false;
        emit disconnected();
    });
    connect(&m_socket, &QWebSocket::textMessageReceived, this, &SignalingClient::handleIncomingMessage);
    connect(&m_socket, &QWebSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        emit errorOccurred(m_socket.errorString());
    });
}

void SignalingClient::setCredentials(const RealtimeCredentials &credentials)
{
    m_credentials = credentials;
}

void SignalingClient::setAppToken(const QString &token)
{
    m_appToken = token;
}

void SignalingClient::connectToRealtime()
{
    if (!m_credentials.endpoint.isValid()) {
        emit errorOccurred(QStringLiteral("Realtime endpoint is invalid"));
        return;
    }

    if (m_socket.state() == QAbstractSocket::ConnectedState || m_socket.state() == QAbstractSocket::ConnectingState) {
        return;
    }

    QNetworkRequest request(m_credentials.endpoint);
    request.setRawHeader("apikey", m_credentials.apiKey.toUtf8());
    if (!m_appToken.isEmpty()) {
        request.setRawHeader("Authorization", QByteArrayLiteral("Bearer ") + m_appToken.toUtf8());
    }

    m_socket.open(request);
}

void SignalingClient::disconnectFromRealtime()
{
    if (m_socket.state() == QAbstractSocket::ConnectedState || m_socket.state() == QAbstractSocket::ConnectingState) {
        m_socket.close();
    }
}

void SignalingClient::sendSignal(const SignalEnvelope &envelope)
{
    if (!m_joined) {
        emit errorOccurred(QStringLiteral("Not joined to realtime topic"));
        return;
    }

    QJsonObject payload = envelope.payload;
    payload.insert(QStringLiteral("type"), envelope.type);

    const QJsonObject message{
        {QStringLiteral("topic"), m_credentials.topic},
        {QStringLiteral("event"), QStringLiteral("signal")},
        {QStringLiteral("payload"), payload},
        {QStringLiteral("ref"), QString::number(QDateTime::currentMSecsSinceEpoch())},
    };

    m_socket.sendTextMessage(QString::fromUtf8(QJsonDocument(message).toJson(QJsonDocument::Compact)));
}

void SignalingClient::reset()
{
    m_joined = false;
}

void SignalingClient::sendJoinMessage()
{
    reset();

    const QJsonObject payload{
        {QStringLiteral("apikey"), m_credentials.apiKey},
        {QStringLiteral("token"), m_credentials.signedToken},
    };

    const QJsonObject joinMessage{
        {QStringLiteral("topic"), m_credentials.topic},
        {QStringLiteral("event"), QStringLiteral("phx_join")},
        {QStringLiteral("payload"), payload},
        {QStringLiteral("ref"), QString::number(QDateTime::currentMSecsSinceEpoch())},
    };

    m_socket.sendTextMessage(QString::fromUtf8(QJsonDocument(joinMessage).toJson(QJsonDocument::Compact)));
    emit connected();
}

void SignalingClient::handleIncomingMessage(const QString &jsonText)
{
    const auto doc = QJsonDocument::fromJson(jsonText.toUtf8());
    if (!doc.isObject()) {
        return;
    }

    const auto obj = doc.object();
    const auto event = obj.value(QStringLiteral("event")).toString();
    if (event == QStringLiteral("phx_reply")) {
        const auto payload = obj.value(QStringLiteral("payload")).toObject();
        const auto status = payload.value(QStringLiteral("status")).toString();
        if (status == QStringLiteral("ok")) {
            m_joined = true;
        }
        return;
    }

    if (event == QStringLiteral("signal")) {
        const auto payload = obj.value(QStringLiteral("payload")).toObject();
        const auto type = payload.value(QStringLiteral("type")).toString();
        QJsonObject data = payload;
        data.remove(QStringLiteral("type"));
        emit signalReceived(SignalEnvelope{type, data});
    }
}

} // namespace controller
