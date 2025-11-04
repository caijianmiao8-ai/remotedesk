#include "host/SignalingClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QUrlQuery>
#include <QDateTime>
#include <QNetworkRequest>

SignalingClient::SignalingClient(QObject* parent)
    : QObject(parent)
{
    connect(&m_socket, &QWebSocket::connected, this, &SignalingClient::onSocketConnected);
    connect(&m_socket, &QWebSocket::textMessageReceived, this, &SignalingClient::onSocketTextMessage);
    connect(&m_socket, &QWebSocket::disconnected, this, &SignalingClient::onSocketClosed);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(&m_socket, &QWebSocket::errorOccurred, this, &SignalingClient::onSocketError);
#else
    connect(&m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
#endif

    m_heartbeat.setInterval(30000); // 30s
    connect(&m_heartbeat, &QTimer::timeout, this, &SignalingClient::onHeartbeat);
}

void SignalingClient::setCredentials(const RealtimeCredentials& cred) { m_cred = cred; }
void SignalingClient::setAppToken(const QString& appToken) { m_appToken = appToken; }

QUrl SignalingClient::buildRealtimeWsUrl(const QUrl& endpoint, const QString& apikey, const QString& token) const {
    // endpoint: https://<project>.supabase.co
    QUrl ws = endpoint;
    if (ws.scheme() == "https") ws.setScheme("wss");
    else if (ws.scheme() == "http") ws.setScheme("ws");
    ws.setPath("/realtime/v1/websocket");

    QUrlQuery q;
    q.addQueryItem("apikey", apikey);
    q.addQueryItem("vsn", "1.0.0");
    if (!token.isEmpty()) q.addQueryItem("token", token);
    ws.setQuery(q);
    return ws;
}

void SignalingClient::connectToRealtime() {
    if (!m_cred.endpoint.isValid() || m_cred.apiKey.isEmpty() || m_cred.topic.isEmpty()) {
        emit errorOccurred(QStringLiteral("Invalid Realtime credentials"));
        return;
    }
    const QUrl wsUrl = buildRealtimeWsUrl(m_cred.endpoint, m_cred.apiKey, m_cred.signedToken);

    QNetworkRequest req(wsUrl);
    if (!m_appToken.isEmpty()) {
        req.setRawHeader("Authorization", QByteArray("Bearer ").append(m_appToken.toUtf8()));
    }
    m_socket.open(req);
}

void SignalingClient::disconnectFromRealtime() {
    m_heartbeat.stop();
    m_socket.close();
}

void SignalingClient::onSocketConnected() {
    emit connected();
    m_joined = false;
    sendJoin();
    m_heartbeat.start();
}

void SignalingClient::sendJoin() {
    // Phoenix: event=phx_join
    const QJsonObject obj{
        {"topic",   m_cred.topic},
        {"event",   "phx_join"},
        {"payload", QJsonObject{}},
        {"ref",     QString::number(m_refCounter++)}
    };
    sendRaw(obj);
}

void SignalingClient::onHeartbeat() {
    const QJsonObject beat{
        {"topic",   "phoenix"},
        {"event",   "heartbeat"},
        {"payload", QJsonObject{}},
        {"ref",     QString::number(m_refCounter++)}
    };
    sendRaw(beat);
}

void SignalingClient::sendBroadcast(const QString& event, const QJsonObject& payload) {
    // 统一使用 broadcast：外层 event="broadcast"，内层 event=你自定义（如 "signal"）
    const QJsonObject obj{
        {"topic",   m_cred.topic},
        {"event",   "broadcast"},
        {"payload", QJsonObject{
            {"type",   "broadcast"},
            {"event",  event},         // e.g. "signal"
            {"payload", payload}       // 自己的内容（offer/answer/ice）
        }},
        {"ref",     QString::number(m_refCounter++)}
    };
    sendRaw(obj);
}

void SignalingClient::sendSignal(const SignalEnvelope& env) {
    QJsonObject inner = env.data;
    inner.insert("type", env.type); // 例如 "offer" / "answer" / "ice"
    sendBroadcast(QStringLiteral("signal"), inner);
}

void SignalingClient::sendRaw(const QJsonObject& obj) {
    const auto str = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    m_socket.sendTextMessage(str);
}

void SignalingClient::onSocketTextMessage(const QString& msg) {
    const auto obj = QJsonDocument::fromJson(msg.toUtf8()).object();
    const QString event = obj.value("event").toString();

    if (event == "phx_reply") {
        // join ok?
        const auto payload = obj.value("payload").toObject();
        const auto status = payload.value("status").toString();
        if (!m_joined && status == "ok" && obj.value("topic").toString() == m_cred.topic) {
            m_joined = true;
            emit joined();
        }
        return;
    }

    if (event == "broadcast") {
        const auto p = obj.value("payload").toObject();
        if (p.value("type").toString() == "broadcast" &&
            p.value("event").toString() == "signal") {
            const auto inner = p.value("payload").toObject(); // 真正信令
            const QString t = inner.value("type").toString();
            QJsonObject data = inner;
            data.remove("type");
            emit signalReceived(SignalEnvelope{t, data});
        }
        return;
    }

    // 其它事件忽略
}

void SignalingClient::onSocketClosed() {
    m_heartbeat.stop();
    emit closed();
}

void SignalingClient::onSocketError(QAbstractSocket::SocketError) {
    emit errorOccurred(m_socket.errorString());
}
