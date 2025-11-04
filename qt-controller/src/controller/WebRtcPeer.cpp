#include "controller/WebRtcPeer.h"

#include "common/Protocol.h"

namespace controller {

WebRtcPeer::WebRtcPeer(QObject *parent)
    : QObject(parent)
{
}

WebRtcPeer::~WebRtcPeer()
{
    closePeer();
}

void WebRtcPeer::setIceServers(const std::vector<IceServer> &servers)
{
    m_iceServers = servers;
}

void WebRtcPeer::createPeer()
{
    rtc::Configuration config;
    for (const auto &server : m_iceServers) {
        rtc::IceServer ice;
        for (const auto &url : server.urls) {
            ice.urls.emplace_back(url.toStdString());
        }
        ice.username = server.username.toStdString();
        ice.password = server.credential.toStdString();
        config.iceServers.emplace_back(ice);
    }

    m_peerConnection = std::make_shared<rtc::PeerConnection>(config);

    m_peerConnection->onLocalDescription([this](const rtc::Description &description) {
        const auto type = QString::fromStdString(description.typeString());
        const auto sdp = QString::fromStdString(description.sdp());
        emit localDescriptionReady(type, sdp);
    });

    m_peerConnection->onLocalCandidate([this](const rtc::Candidate &candidate) {
        const auto candidateSdp = QString::fromStdString(candidate.candidate());
        const auto sdpMid = QString::fromStdString(candidate.mid());
        const int mline = candidate.mlineindex();
        emit localIceCandidate(candidateSdp, sdpMid, mline);
    });

    m_peerConnection->onStateChange([this](rtc::PeerConnection::State state) {
        QString text;
        switch (state) {
        case rtc::PeerConnection::State::New:
            text = QStringLiteral("new");
            break;
        case rtc::PeerConnection::State::Connecting:
            text = QStringLiteral("connecting");
            break;
        case rtc::PeerConnection::State::Connected:
            text = QStringLiteral("connected");
            break;
        case rtc::PeerConnection::State::Disconnected:
            text = QStringLiteral("disconnected");
            break;
        case rtc::PeerConnection::State::Failed:
            text = QStringLiteral("failed");
            break;
        case rtc::PeerConnection::State::Closed:
            text = QStringLiteral("closed");
            break;
        }
        emit stateChanged(text);
    });

    m_peerConnection->onGatheringStateChange([this](rtc::PeerConnection::GatheringState state) {
        if (state == rtc::PeerConnection::GatheringState::Complete) {
            emit stateChanged(QStringLiteral("ice-complete"));
        }
    });

    m_peerConnection->onTrack([this](rtc::Track::Ptr track) {
        m_tracks.push_back(track);
    });

    m_inputChannel = m_peerConnection->createDataChannel(Protocol::kInputChannelName);

    attachMediaHandlers();
}

void WebRtcPeer::closePeer()
{
    if (m_inputChannel) {
        m_inputChannel->close();
        m_inputChannel.reset();
    }

    if (m_peerConnection) {
        m_peerConnection->close();
        m_peerConnection.reset();
    }

    m_tracks.clear();
}

void WebRtcPeer::createOffer()
{
    if (!m_peerConnection) {
        return;
    }

    m_peerConnection->setLocalDescription();
}

void WebRtcPeer::setRemoteDescription(const QString &type, const QString &sdp)
{
    if (!m_peerConnection) {
        return;
    }

    rtc::Description description(sdp.toStdString(), type.toStdString());
    m_peerConnection->setRemoteDescription(description);
}

void WebRtcPeer::addRemoteIceCandidate(const QString &candidate, const QString &sdpMid, int sdpMLineIndex)
{
    if (!m_peerConnection) {
        return;
    }

    rtc::Candidate iceCandidate(candidate.toStdString(), sdpMid.toStdString(), sdpMLineIndex);
    m_peerConnection->addRemoteCandidate(iceCandidate);
}

void WebRtcPeer::sendInputEvent(const QByteArray &payload)
{
    if (m_inputChannel && m_inputChannel->isOpen()) {
        m_inputChannel->send(std::string(payload.constData(), static_cast<std::size_t>(payload.size())));
    }
}

void WebRtcPeer::attachMediaHandlers()
{
    // Placeholder for future video/audio rendering logic.
}

} // namespace controller
