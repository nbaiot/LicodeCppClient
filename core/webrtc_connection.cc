//
// Created by nbaiot@126.com on 2020/10/15.
//

#include "webrtc_connection.h"
#include "webrtc_wrapper.h"
#include "video_render.h"

#include <glog/logging.h>

namespace nbaiot {

/// TODO: fixme
class DummySetSessionDescriptionObserver
    : public webrtc::SetSessionDescriptionObserver {
public:
  static DummySetSessionDescriptionObserver* Create(bool remote = false) {
    return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>(remote);
  }

  void OnSuccess() override {
    LOG(INFO) << ">>>>>>>>>>>>>>>>>> SetSessionDescription "
              << (remote_ ? "remote" : "local") << " sdp success";
  }

  void OnFailure(webrtc::RTCError error) override {
    LOG(ERROR) << ">>>>>>>>>>>>>>>>>> SetSessionDescription "
               << (remote_ ? "remote" : "local") << " sdp failed."
               << webrtc::ToString(error.type()) << ": "
               << error.message();
  }

protected:
  explicit DummySetSessionDescriptionObserver(bool remote) : remote_(remote) {}

private:
  bool remote_;
};

WebrtcConnection::WebrtcConnection(const webrtc::PeerConnectionInterface::RTCConfiguration& config)
    : rtc_config_(config) {
  auto pcFactory = WebrtcWrapper::Instance()->CreatePCFactory();
  pc_ = pcFactory->CreatePeerConnection(config, nullptr, nullptr, this);
}

void WebrtcConnection::SetRemoteDescription(std::unique_ptr<webrtc::SessionDescriptionInterface> desc) {
  pc_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(true), desc.release());
}

void WebrtcConnection::SetLocalDescription(std::unique_ptr<webrtc::SessionDescriptionInterface> desc) {
  pc_->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc.release());
}

void WebrtcConnection::CreateOffer(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions& options) {
  /// The CreateSessionDescriptionObserver callback will be called when done
  pc_->CreateAnswer(this, options);
}

void WebrtcConnection::CreateAnswer(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions& options) {
  /// The CreateSessionDescriptionObserver callback will be called when done.
  pc_->CreateAnswer(this, options);
}

void WebrtcConnection::SetSdpCreateSuccessCallback(WebrtcConnection::OnSdpCreateSuccessCallback callback) {
  sdp_create_success_callback_ = std::move(callback);
}

void WebrtcConnection::SetIceCandidateCallback(WebrtcConnection::OnIceCandidateCallback callback) {
  ice_candidate_callback_ = std::move(callback);
}

void WebrtcConnection::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
  std::string out;
  desc->ToString(&out);
  LOG(INFO) << ">>>>>>WebrtcConnection OnSuccess:" << desc->type() << "\n" << out;
  if (sdp_create_success_callback_) {
    sdp_create_success_callback_(desc->GetType(), out);
  }
}

void WebrtcConnection::OnFailure(webrtc::RTCError error) {
  LOG(ERROR) << ">>>>>>WebrtcConnection OnFailure:" << error.message();
}

void WebrtcConnection::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {
  LOG(INFO) << ">>>>>>WebrtcConnection OnSignalingChange:" << WebrtcWrapper::SignalingStateToString(new_state);
}

void WebrtcConnection::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {
  LOG(INFO) << ">>>>>>WebrtcConnection OnDataChannel label:" << data_channel->label();
}

void WebrtcConnection::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
  LOG(INFO) << ">>>>>>WebrtcConnection OnIceGatheringChange:" << WebrtcWrapper::IceGatheringStateToString(new_state);
}

void WebrtcConnection::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  std::string out;
  candidate->ToString(&out);
  LOG(INFO) << ">>>>>>WebrtcConnection OnIceCandidate:" << out;
  if (ice_candidate_callback_) {
    ice_candidate_callback_(candidate);
  }
}

void WebrtcConnection::OnRenegotiationNeeded() {
  LOG(INFO) << ">>>>>>WebrtcConnection OnRenegotiationNeeded";
}

void WebrtcConnection::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
  LOG(INFO) << ">>>>>>WebrtcConnection OnIceConnectionChange:" << WebrtcWrapper::IceConnectionStateToString(new_state);
}

void WebrtcConnection::OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) {
  LOG(INFO) << ">>>>>>WebrtcConnection OnConnectionChange:"
            << WebrtcWrapper::PeerConnectionStateToString(static_cast<int>(new_state));
}

void WebrtcConnection::OnIceConnectionReceivingChange(bool receiving) {
  LOG(INFO) << ">>>>>>WebrtcConnection OnIceConnectionReceivingChange:" << receiving;
}

void WebrtcConnection::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
                                  const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) {
  LOG(INFO) << ">>>>>>WebrtcConnection OnAddTrack " << "\n"
            << "media type:" << receiver->media_type() << "\n"
            << "track id:" << receiver->track()->id() << "\n"
            << "track kind:" << receiver->track()->kind() << "\n";
  /// TODO: fixme
  if (receiver->track()->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
    auto* video_track = static_cast<webrtc::VideoTrackInterface*>(receiver->track().get());
    video_track->AddOrUpdateSink(new VideoRender(), rtc::VideoSinkWants());
  }
}

void WebrtcConnection::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
  /// This will only be called if Unified Plan semantics are specified
  LOG(INFO) << ">>>>>>WebrtcConnection OnTrack";
}

void WebrtcConnection::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
  LOG(INFO) << ">>>>>>WebrtcConnection OnRemoveTrack " << "\n"
            << "media type:" << receiver->media_type() << "\n"
            << "track id:" << receiver->track()->id() << "\n"
            << "track kind:" << receiver->track()->kind() << "\n";
}

rtc::scoped_refptr<webrtc::PeerConnectionInterface> WebrtcConnection::PeerConnection() {
  return pc_;
}

}