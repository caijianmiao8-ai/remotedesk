#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <QByteArray>
#include <QImage>
#include <QObject>
#include <QString>
#include <QStringList>

#include <rtc/rtc.hpp>

namespace controller {

struct IceServer
{
    QStringList urls;
    QString username;
    QString credential;
};

class WebRtcPeer : public QObject
{
    Q_OBJECT

public:
    explicit WebRtcPeer(QObject *parent = nullptr);
    ~WebRtcPeer() override;

    void setIceServers(const std::vector<IceServer> &servers);
    void createPeer();
    void closePeer();

    void createOffer();
    void setRemoteDescription(const QString &type, const QString &sdp);
    void addRemoteIceCandidate(const QString &candidate, const QString &sdpMid, int sdpMLineIndex);
    void sendInputEvent(const QByteArray &payload);

signals:
    void localDescriptionReady(const QString &type, const QString &sdp);
    void localIceCandidate(const QString &candidate, const QString &sdpMid, int sdpMLineIndex);
    void stateChanged(const QString &newState);
    void videoFrameReady(const QImage &frame);

private:
    void attachMediaHandlers();

    std::vector<IceServer> m_iceServers;
    std::shared_ptr<rtc::PeerConnection> m_peerConnection;
    std::shared_ptr<rtc::DataChannel> m_inputChannel;
    std::vector<std::shared_ptr<rtc::Track>> m_tracks;
};

} // namespace controller
