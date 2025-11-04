#include "controller/WebRtcPeer.h"

#include "common/Protocol.h"

#include <cstdint>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

template <typename> struct AlwaysFalse : std::false_type {};

template <typename T, typename = void>
struct HasSdpMethod : std::false_type {};

template <typename T>
struct HasSdpMethod<T, std::void_t<decltype(std::declval<const T &>().sdp())>> : std::true_type {};

template <typename T, typename = void>
struct HasSerializeMethod : std::false_type {};

template <typename T>
struct HasSerializeMethod<T, std::void_t<decltype(std::declval<const T &>().serialize())>> : std::true_type {};

template <typename T, typename = void>
struct HasToStringMethod : std::false_type {};

template <typename T>
struct HasToStringMethod<T, std::void_t<decltype(std::declval<const T &>().toString())>> : std::true_type {};

template <typename T, typename = void>
struct HasStrMethod : std::false_type {};

template <typename T>
struct HasStrMethod<T, std::void_t<decltype(std::declval<const T &>().str())>> : std::true_type {};

template <typename T, typename = void>
struct HasStringMethod : std::false_type {};

template <typename T>
struct HasStringMethod<T, std::void_t<decltype(std::declval<const T &>().string())>> : std::true_type {};

template <typename T, typename = void>
struct HasGenerateMethod : std::false_type {};

template <typename T>
struct HasGenerateMethod<T, std::void_t<decltype(std::declval<const T &>().generate())>> : std::true_type {};

template <typename T, typename = void>
struct HasOStreamOperator : std::false_type {};

template <typename T>
struct HasOStreamOperator<T, std::void_t<decltype(std::declval<std::ostream &>() << std::declval<const T &>())>> : std::true_type {};

template <typename T>
constexpr bool IsStringConvertibleV = std::is_convertible_v<T, std::string>;

template <typename T>
QString descriptionSdp(const T &description)
{
    if constexpr (HasSdpMethod<T>::value) {
        return QString::fromStdString(description.sdp());
    } else if constexpr (HasSerializeMethod<T>::value) {
        return QString::fromStdString(description.serialize());
    } else if constexpr (HasToStringMethod<T>::value) {
        return QString::fromStdString(description.toString());
    } else if constexpr (HasStrMethod<T>::value) {
        return QString::fromStdString(description.str());
    } else if constexpr (HasStringMethod<T>::value) {
        return QString::fromStdString(description.string());
    } else if constexpr (HasGenerateMethod<T>::value) {
        return QString::fromStdString(description.generate());
    } else if constexpr (IsStringConvertibleV<T>) {
        return QString::fromStdString(static_cast<std::string>(description));
    } else if constexpr (HasOStreamOperator<T>::value) {
        std::ostringstream oss;
        oss << description;
        return QString::fromStdString(oss.str());
    } else {
        static_assert(AlwaysFalse<T>::value, "Unsupported rtc::Description API");
    }
}

template <typename T>
struct IsOptional : std::false_type {};

template <typename U>
struct IsOptional<std::optional<U>> : std::true_type {};

template <typename T, typename = void>
struct HasMidMethod : std::false_type {};

template <typename T>
struct HasMidMethod<T, std::void_t<decltype(std::declval<const T &>().mid())>> : std::true_type {};

template <typename T, typename = void>
struct HasSdpMidMethod : std::false_type {};

template <typename T>
struct HasSdpMidMethod<T, std::void_t<decltype(std::declval<const T &>().sdpMid())>> : std::true_type {};

template <typename T, typename = void>
struct HasMLineIndexMethod : std::false_type {};

template <typename T>
struct HasMLineIndexMethod<T, std::void_t<decltype(std::declval<const T &>().mlineindex())>> : std::true_type {};

template <typename T, typename = void>
struct HasSdpMLineIndexMethod : std::false_type {};

template <typename T>
struct HasSdpMLineIndexMethod<T, std::void_t<decltype(std::declval<const T &>().sdpMLineIndex())>> : std::true_type {};

template <typename T, typename = void>
struct HasCamelMLineIndexMethod : std::false_type {};

template <typename T>
struct HasCamelMLineIndexMethod<T, std::void_t<decltype(std::declval<const T &>().mLineIndex())>> : std::true_type {};

template <typename T>
std::optional<std::string> normalizeCandidateMid(T &&value)
{
    using ValueType = std::decay_t<T>;
    if constexpr (IsOptional<ValueType>::value) {
        if (value) {
            return *value;
        }
        return std::nullopt;
    } else if constexpr (std::is_convertible_v<ValueType, std::string>) {
        auto str = std::string(std::forward<T>(value));
        if (str.empty()) {
            return std::nullopt;
        }
        return str;
    } else {
        static_assert(AlwaysFalse<ValueType>::value, "Unsupported candidate mid type");
    }
}

template <typename T>
std::optional<int> normalizeCandidateIndex(T &&value)
{
    using ValueType = std::decay_t<T>;
    if constexpr (IsOptional<ValueType>::value) {
        if (value) {
            return static_cast<int>(*value);
        }
        return std::nullopt;
    } else if constexpr (std::is_convertible_v<ValueType, int>) {
        return static_cast<int>(std::forward<T>(value));
    } else if constexpr (std::is_convertible_v<ValueType, std::uint16_t>) {
        return static_cast<int>(std::forward<T>(value));
    } else {
        static_assert(AlwaysFalse<ValueType>::value, "Unsupported candidate index type");
    }
}

template <typename Candidate>
std::optional<std::string> candidateMid(const Candidate &candidate)
{
    if constexpr (HasMidMethod<Candidate>::value) {
        return normalizeCandidateMid(candidate.mid());
    } else if constexpr (HasSdpMidMethod<Candidate>::value) {
        return normalizeCandidateMid(candidate.sdpMid());
    } else {
        return std::nullopt;
    }
}

template <typename Candidate>
std::optional<int> candidateMLineIndex(const Candidate &candidate)
{
    if constexpr (HasMLineIndexMethod<Candidate>::value) {
        return normalizeCandidateIndex(candidate.mlineindex());
    } else if constexpr (HasSdpMLineIndexMethod<Candidate>::value) {
        return normalizeCandidateIndex(candidate.sdpMLineIndex());
    } else if constexpr (HasCamelMLineIndexMethod<Candidate>::value) {
        return normalizeCandidateIndex(candidate.mLineIndex());
    } else {
        return std::nullopt;
    }
}

template <typename IceServerT>
IceServerT makeIceServer(const QStringList &urls, const QString &username, const QString &password)
{
    std::vector<std::string> urlList;
    urlList.reserve(urls.size());
    for (const auto &url : urls) {
        urlList.emplace_back(url.toStdString());
    }

    const std::string user = username.toStdString();
    const std::string pass = password.toStdString();

    if constexpr (std::is_default_constructible_v<IceServerT>) {
        IceServerT server;
        server.urls = std::move(urlList);
        server.username = user;
        server.password = pass;
        return server;
    } else if constexpr (std::is_constructible_v<IceServerT, std::vector<std::string>, std::string, std::string>) {
        return IceServerT(std::move(urlList), user, pass);
    } else if constexpr (std::is_constructible_v<IceServerT, std::vector<std::string>>) {
        return IceServerT(std::move(urlList));
    } else if constexpr (std::is_constructible_v<IceServerT, std::vector<std::string>, std::optional<std::string>>) {
        std::optional<std::string> userOpt = username.isEmpty() ? std::nullopt : std::make_optional(user);
        return IceServerT(std::move(urlList), std::move(userOpt));
    } else if constexpr (std::is_constructible_v<IceServerT, std::vector<std::string>, std::optional<std::string>, std::optional<std::string>>) {
        std::optional<std::string> userOpt = username.isEmpty() ? std::nullopt : std::make_optional(user);
        std::optional<std::string> passOpt = password.isEmpty() ? std::nullopt : std::make_optional(pass);
        return IceServerT(std::move(urlList), std::move(userOpt), std::move(passOpt));
    } else {
        static_assert(AlwaysFalse<IceServerT>::value, "Unsupported rtc::IceServer constructor");
    }
}

template <typename CandidateT>
CandidateT makeCandidate(const QString &candidate, const QString &sdpMid, int sdpMLineIndex)
{
    const std::string candidateStr = candidate.toStdString();
    const bool hasMid = !sdpMid.isEmpty();
    const bool hasMLine = sdpMLineIndex >= 0;
    const std::string midStr = sdpMid.toStdString();

    if constexpr (std::is_constructible_v<CandidateT, std::string, std::string, int>) {
        return CandidateT(candidateStr, hasMid ? midStr : std::string(), hasMLine ? sdpMLineIndex : 0);
    } else if constexpr (std::is_constructible_v<CandidateT, std::string, std::optional<std::string>, std::optional<int>>) {
        return CandidateT(candidateStr,
                          hasMid ? std::make_optional(midStr) : std::nullopt,
                          hasMLine ? std::make_optional(sdpMLineIndex) : std::nullopt);
    } else if constexpr (std::is_constructible_v<CandidateT, std::string, std::optional<std::string>, std::optional<std::uint16_t>>) {
        return CandidateT(candidateStr,
                          hasMid ? std::make_optional(midStr) : std::nullopt,
                          hasMLine ? std::make_optional(static_cast<std::uint16_t>(sdpMLineIndex)) : std::nullopt);
    } else {
        static_assert(AlwaysFalse<CandidateT>::value, "Unsupported rtc::Candidate constructor");
    }
}

} // namespace

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
        using IceServerType = typename decltype(config.iceServers)::value_type;
        config.iceServers.emplace_back(makeIceServer<IceServerType>(server.urls, server.username, server.credential));
    }

    m_peerConnection = std::make_shared<rtc::PeerConnection>(config);

    m_peerConnection->onLocalDescription([this](const rtc::Description &description) {
        const auto type = QString::fromStdString(description.typeString());
        const auto sdp = descriptionSdp(description);
        emit localDescriptionReady(type, sdp);
    });

    m_peerConnection->onLocalCandidate([this](const rtc::Candidate &candidate) {
        const auto candidateSdp = QString::fromStdString(candidate.candidate());
        const auto midOpt = candidateMid(candidate);
        const auto mlineOpt = candidateMLineIndex(candidate);
        const QString sdpMid = midOpt ? QString::fromStdString(*midOpt) : QString();
        const int mline = mlineOpt.value_or(-1);
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

    m_peerConnection->onTrack([this](std::shared_ptr<rtc::Track> track) {
        m_tracks.push_back(std::move(track));
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

    using CandidateType = rtc::Candidate;
    auto iceCandidate = makeCandidate<CandidateType>(candidate, sdpMid, sdpMLineIndex);
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
