//
// Created by nbaiot@126.com on 2020/10/15.
//

#ifndef LICODECPPCLIENT_WEBRTC_CONNECTION_H
#define LICODECPPCLIENT_WEBRTC_CONNECTION_H

#include <functional>

#include "api/peer_connection_interface.h"

namespace nbaiot {

/// TODO: 对外提供 observer 代替大量单一设置 callback

class WebrtcConnection : public webrtc::PeerConnectionObserver,
                         public webrtc::CreateSessionDescriptionObserver {

public:

  using OnSdpCreateSuccessCallback = std::function<void(webrtc::SdpType type, const std::string& sdp)>;

  using OnIceCandidateCallback = std::function<void(const webrtc::IceCandidateInterface* candidate)>;

  explicit WebrtcConnection(const webrtc::PeerConnectionInterface::RTCConfiguration& config);

  void SetLocalDescription(std::unique_ptr<webrtc::SessionDescriptionInterface> desc);

  void SetRemoteDescription(std::unique_ptr<webrtc::SessionDescriptionInterface> desc);

  void CreateOffer(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions& options);

  void CreateAnswer(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions& options);

  void SetSdpCreateSuccessCallback(OnSdpCreateSuccessCallback callback);

  void SetIceCandidateCallback(OnIceCandidateCallback callback);

  /// TODO: add AttachVideoRenders(多个 render) and AttachAudioRender


  /// sdp observer
  void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;

  void OnFailure(webrtc::RTCError error) override;

  /// pc observer
  void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;

  void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;

  void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;

  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;

  void OnRenegotiationNeeded() override;

  void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;

  void OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) override;

  void OnIceConnectionReceivingChange(bool receiving) override;

  void OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
                  const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) override;

  /// Note: This will only be called if Unified Plan semantics are specified.
  void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override;

  void OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;

  /// TODO: only for test
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> PeerConnection();

private:
  webrtc::PeerConnectionInterface::RTCConfiguration rtc_config_;
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc_;

  OnSdpCreateSuccessCallback sdp_create_success_callback_;
  OnIceCandidateCallback ice_candidate_callback_;
};

}

#endif //LICODECPPCLIENT_WEBRTC_CONNECTION_H
