//
// Created by nbaiot@126.com on 2020/10/15.
//

#include "webrtc_wrapper.h"

#include "rtc_base/thread.h"
#include "rtc_base/logging.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "pc/peer_connection_factory.h"
#include "api/peer_connection_interface.h"

namespace nbaiot {

WebrtcWrapper* WebrtcWrapper::Instance() {
  static WebrtcWrapper INSTANCE;
  return &INSTANCE;
}

void WebrtcWrapper::Init() {
  if (init_)
    return;
  init_ = true;
  rtc::LogMessage::LogTimestamps(true);
  rtc::LogMessage::SetLogToStderr(true);
  rtc::LogMessage::LogThreads(true);
  signaling_thread_ = rtc::Thread::Create();
  signaling_thread_->Start();
}

void WebrtcWrapper::SetWebrtcLogLevel(WebrtcWrapper::WebrtcLogLevel level) {
  switch (level) {
    case kVerbose:
      rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
      break;
    case kInfo:
      rtc::LogMessage::LogToDebug(rtc::LS_INFO);
      break;
    case kWarning:
      rtc::LogMessage::LogToDebug(rtc::LS_WARNING);
      break;
    case kError:
      rtc::LogMessage::LogToDebug(rtc::LS_ERROR);
      break;
    case kNone:
      rtc::LogMessage::LogToDebug(rtc::LS_NONE);
      break;
    default:
      rtc::LogMessage::LogToDebug(rtc::LS_WARNING);
  }

}

rtc::Thread* WebrtcWrapper::SignalingThread() {
  return signaling_thread_.get();
}

rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> WebrtcWrapper::CreatePCFactory() {
  if (!init_)
    return nullptr;
  return webrtc::CreatePeerConnectionFactory(nullptr,
                                             nullptr,
                                             SignalingThread(),
                                             nullptr,
                                             webrtc::CreateBuiltinAudioEncoderFactory(),
                                             webrtc::CreateBuiltinAudioDecoderFactory(),
                                             webrtc::CreateBuiltinVideoEncoderFactory(),
                                             webrtc::CreateBuiltinVideoDecoderFactory(),
                                             nullptr, nullptr);
}

std::unique_ptr<webrtc::SessionDescriptionInterface>
WebrtcWrapper::CreateSessionDescription(webrtc::SdpType type, const std::string& sdp,
                                        webrtc::SdpParseError* error_out) {
  return webrtc::CreateSessionDescription(type, sdp, error_out);
}

std::string WebrtcWrapper::SignalingStateToString(int state) {
  switch (state) {
    case webrtc::PeerConnectionInterface::SignalingState::kStable:
      return "Stable";
    case webrtc::PeerConnectionInterface::SignalingState::kHaveLocalOffer:
      return "HaveLocalOffer";
    case webrtc::PeerConnectionInterface::SignalingState::kHaveLocalPrAnswer:
      return "HaveLocalPrAnswer";
    case webrtc::PeerConnectionInterface::SignalingState::kHaveRemoteOffer:
      return "HaveRemoteOffer";
    case webrtc::PeerConnectionInterface::SignalingState::kHaveRemotePrAnswer:
      return "HaveRemotePrAnswer";
    case webrtc::PeerConnectionInterface::SignalingState::kClosed:
      return "Closed";
    default:
      return "Unknown";
  }
}

std::string WebrtcWrapper::IceGatheringStateToString(int state) {
  switch (state) {
    case webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringNew:
      return "IceGatheringNew";
    case webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringGathering:
      return "IceGatheringGathering";
    case webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete:
      return "IceGatheringComplete";
    default:
      return "Unknown";
  }
}

std::string WebrtcWrapper::IceConnectionStateToString(int state) {
  switch (state) {
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionNew:
      return "IceConnectionNew";
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionChecking:
      return "IceConnectionChecking";
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionConnected:
      return "IceConnectionConnected";
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionCompleted:
      return "IceConnectionConnected";
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionFailed:
      return "IceConnectionFailed";
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionDisconnected:
      return "IceConnectionDisconnected";
    case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed:
      return "IceConnectionClosed";
    default:
      return "Unknown";
  }

}

std::string WebrtcWrapper::PeerConnectionStateToString(int state) {
  switch (state) {
    case static_cast<int>(webrtc::PeerConnectionInterface::PeerConnectionState::kNew):
      return "New";
    case static_cast<int>(webrtc::PeerConnectionInterface::PeerConnectionState::kConnecting):
      return "Connecting";
    case static_cast<int>(webrtc::PeerConnectionInterface::PeerConnectionState::kConnected):
      return "Connected";
    case static_cast<int>(webrtc::PeerConnectionInterface::PeerConnectionState::kDisconnected):
      return "Disconnected";
    case static_cast<int>(webrtc::PeerConnectionInterface::PeerConnectionState::kFailed):
      return "Failed";
    case static_cast<int>(webrtc::PeerConnectionInterface::PeerConnectionState::kClosed):
      return "Closed";
    default:
      return "Unknown";
  }
}

}
