//
// Created by nbaiot@126.com on 2020/10/15.
//

#ifndef LICODECPPCLIENT_WEBRTC_CONNECTION_H
#define LICODECPPCLIENT_WEBRTC_CONNECTION_H

#include <functional>

#include "api/peer_connection_interface.h"

namespace nbaiot {

class WebrtcConnection : public webrtc::PeerConnectionObserver,
                         public webrtc::CreateSessionDescriptionObserver {

public:

  using OnSdpCreateSuccessCallback = std::function<void(webrtc::SdpType type, const std::string& sdp)>;

  explicit WebrtcConnection(const webrtc::PeerConnectionInterface::RTCConfiguration& config);

  void SetRemoteDescription(std::unique_ptr<webrtc::SessionDescriptionInterface> desc);

  void CreateAnswer(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions& options);

  void SetSdpCreateSuccessCallback(OnSdpCreateSuccessCallback callback);

  /// sdp observer
  void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;

  void OnFailure(webrtc::RTCError error) override;

  /// pc observer
  void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;

  void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;

  void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;

  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;

  void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

  void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

  void OnRenegotiationNeeded() override;

  void OnNegotiationNeededEvent(uint32_t event_id) override;

  void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;

  void OnStandardizedIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;

  void OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) override;

  void OnIceCandidateError(const std::string& host_candidate, const std::string& url, int error_code,
                           const std::string& error_text) override;

  void OnIceCandidateError(const std::string& address, int port, const std::string& url, int error_code,
                           const std::string& error_text) override;

  void OnIceCandidatesRemoved(const std::vector<cricket::Candidate>& candidates) override;

  void OnIceConnectionReceivingChange(bool receiving) override;

  void OnIceSelectedCandidatePairChanged(const cricket::CandidatePairChangeEvent& event) override;

  void OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
                  const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) override;

  void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override;

  void OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;

private:
  webrtc::PeerConnectionInterface::RTCConfiguration rtc_config_;
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc_;

  OnSdpCreateSuccessCallback sdp_create_success_callback_;
};

}

#endif //LICODECPPCLIENT_WEBRTC_CONNECTION_H
