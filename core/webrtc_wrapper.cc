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
WebrtcWrapper::CreateSessionDescription(webrtc::SdpType type, const std::string& sdp, webrtc::SdpParseError* error_out) {
  return webrtc::CreateSessionDescription(type, sdp, error_out);
}

}