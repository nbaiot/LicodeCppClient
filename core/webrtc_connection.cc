//
// Created by nbaiot@126.com on 2020/10/15.
//

#include "webrtc_connection.h"
#include "webrtc_wrapper.h"

#include <glog/logging.h>

namespace nbaiot {

/// TODO: fixme
class DummySetSessionDescriptionObserver
    : public webrtc::SetSessionDescriptionObserver {
public:
  static DummySetSessionDescriptionObserver* Create() {
    return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
  }

  void OnSuccess() override {
    LOG(INFO) << ">>>>>>>>>>>>>>>>>>=========== SetSessionDescription success";
  }

  void OnFailure(webrtc::RTCError error) override {
    LOG(ERROR) << ">>>>>>>>>>>>>>>>>>=========== SetSessionDescription failed,"
               << webrtc::ToString(error.type()) << ": "
               << error.message();
  }
};

WebrtcConnection::WebrtcConnection(const webrtc::PeerConnectionInterface::RTCConfiguration& config)
    : rtc_config_(config) {
  auto pcFactory = WebrtcWrapper::Instance()->CreatePCFactory();
  pc_ = pcFactory->CreatePeerConnection(config, nullptr, nullptr, this);
}

void WebrtcConnection::SetRemoteDescription(std::unique_ptr<webrtc::SessionDescriptionInterface> desc) {
  pc_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), desc.release());
}

void WebrtcConnection::CreateAnswer(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions& options) {
  pc_->CreateAnswer(this, options);
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
  LOG(INFO) << ">>>>>>WebrtcConnection OnFailure:" << error.message();
}

void WebrtcConnection::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {
  LOG(INFO) << ">>>>>>======================================WebrtcConnection OnSignalingChange:" << new_state;
}

void WebrtcConnection::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {

}

void WebrtcConnection::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
  LOG(INFO) << ">>>>>>=====================================WebrtcConnection OnIceGatheringChange:" << new_state;
}

void WebrtcConnection::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  std::string out;
  candidate->ToString(&out);
  LOG(INFO) << ">>>>>>==============================WebrtcConnection OnIceCandidate:" << out;
}

void WebrtcConnection::SetSdpCreateSuccessCallback(WebrtcConnection::OnSdpCreateSuccessCallback callback) {
  sdp_create_success_callback_ = std::move(callback);
}

void WebrtcConnection::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
  LOG(INFO) << ">>>>>>==============================WebrtcConnection OnAddStream";
}

void WebrtcConnection::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
  LOG(INFO) << ">>>>>>==============================WebrtcConnection OnRemoveStream";
}

void WebrtcConnection::OnRenegotiationNeeded() {
  LOG(INFO) << ">>>>>>==============================WebrtcConnection OnRenegotiationNeeded";
}

void WebrtcConnection::OnNegotiationNeededEvent(uint32_t event_id) {
  LOG(INFO) << ">>>>>>==============================WebrtcConnection OnNegotiationNeededEvent";
}

void WebrtcConnection::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
  LOG(INFO) << ">>>>>>==============================WebrtcConnection OnIceConnectionChange:" << new_state;
}

void
WebrtcConnection::OnStandardizedIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
  LOG(INFO) << ">>>>>>==============================WebrtcConnection OnStandardizedIceConnectionChange:" << new_state;
}

void WebrtcConnection::OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) {
  LOG(INFO) << ">>>>>>==============================WebrtcConnection OnConnectionChange:" << (int)(new_state);
}

void WebrtcConnection::OnIceCandidateError(const std::string& host_candidate, const std::string& url, int error_code,
                                           const std::string& error_text) {
  LOG(INFO) << ">>>>>>==============================WebrtcConnection OnIceCandidateError:" << error_code;
}

void WebrtcConnection::OnIceCandidateError(const std::string& address, int port, const std::string& url, int error_code,
                                           const std::string& error_text) {
  LOG(INFO) << ">>>>>>==============22================WebrtcConnection OnIceCandidateError:" << error_code;
}

void WebrtcConnection::OnIceCandidatesRemoved(const std::vector<cricket::Candidate>& candidates) {
  LOG(INFO) << ">>>>>>==============================WebrtcConnection OnIceCandidatesRemoved:" << candidates.size();
}

void WebrtcConnection::OnIceConnectionReceivingChange(bool receiving) {
  LOG(INFO) << ">>>>>>==============================WebrtcConnection OnIceConnectionReceivingChange:" << receiving;
}

void WebrtcConnection::OnIceSelectedCandidatePairChanged(const cricket::CandidatePairChangeEvent& event) {
  LOG(INFO) << ">>>>>>==============================WebrtcConnection OnIceSelectedCandidatePairChanged";
}

void WebrtcConnection::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
                                  const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) {
  LOG(INFO) << ">>>>>>==============================WebrtcConnection OnAddTrack";
}

void WebrtcConnection::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
  LOG(INFO) << ">>>>>>==============================WebrtcConnection OnTrack";
}

void WebrtcConnection::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
  LOG(INFO) << ">>>>>>==============================WebrtcConnection OnRemoveTrack";
}



}