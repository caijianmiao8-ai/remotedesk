#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QUrl>
#include <QWebSocket>

namespace controller {

struct RealtimeCredentials
{
    QUrl endpoint;
    QString apiKey;
    QString topic;
    QString signedToken;
    QDateTime expiresAt;
};

struct SignalEnvelope
{
    QString type;
    QJsonObject payload;
};

class SignalingClient : public QObject
{
    Q_OBJECT

public:
    explicit SignalingClient(QObject *parent = nullptr);

    void setCredentials(const RealtimeCredentials &credentials);
    void setAppToken(const QString &token);

public slots:
    void connectToRealtime();
    void disconnectFromRealtime();
    void sendSignal(const controller::SignalEnvelope &envelope);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &message);
    void signalReceived(const controller::SignalEnvelope &envelope);

private:
    void reset();
    void sendJoinMessage();
    void handleIncomingMessage(const QString &jsonText);

    RealtimeCredentials m_credentials;
    QString m_appToken;
    QWebSocket m_socket;
    bool m_joined = false;
};

} // namespace controller
