//
// Created by nbaiot@126.com on 2020/10/15.
//

#include "webrtc_connection.h"
#include "webrtc_wrapper.h"

namespace nbaiot {

/// TODO: fixme
class DummySetSessionDescriptionObserver
    : public webrtc::SetSessionDescriptionObserver {
public:
  static DummySetSessionDescriptionObserver* Create() {
    return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
  }

  void OnSuccess() override { RTC_LOG(INFO) << __FUNCTION__; }

  void OnFailure(webrtc::RTCError error) override {
    RTC_LOG(INFO) << __FUNCTION__ << " " << webrtc::ToString(error.type()) << ": "
                  << error.message();
  }
};

WebrtcConnection::WebrtcConnection(const webrtc::PeerConnectionInterface::RTCConfiguration& config)
    : rtc_config_(config) {
  auto pcFactory = WebrtcWrapper::Instance()->CreatePCFactory();
  pc_ = pcFactory->CreatePeerConnection(config, nullptr, nullptr, this);
}

void WebrtcConnection::OnSuccess(webrtc::SessionDescriptionInterface* desc) {

}

void WebrtcConnection::OnFailure(webrtc::RTCError error) {

}

void WebrtcConnection::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {

}

void WebrtcConnection::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {

}

void WebrtcConnection::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {

}

void WebrtcConnection::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {

}

}