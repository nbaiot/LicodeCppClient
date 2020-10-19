//
// Created by nbaiot@126.com on 2020/10/15.
//

#ifndef LICODECPPCLIENT_WEBRTC_CONNECTION_H
#define LICODECPPCLIENT_WEBRTC_CONNECTION_H

#include <boost/noncopyable.hpp>
#include "api/peer_connection_interface.h"
#include "webrtc_peer_connection_observer.h"

namespace nbaiot {

class WebrtcConnection : public webrtc::PeerConnectionObserver,
                         public boost::noncopyable {

public:

  WebrtcConnection(uint64_t id, WebrtcPeerConnectionObserver* observer,
                   const webrtc::PeerConnectionInterface::RTCConfiguration& config);

  uint64_t Id() {
    return id_;
  }

  void SetLocalDescription(std::unique_ptr<webrtc::SessionDescriptionInterface> desc);

  void SetRemoteDescription(std::unique_ptr<webrtc::SessionDescriptionInterface> desc);

  void CreateOffer(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions& options);

  void CreateAnswer(const webrtc::PeerConnectionInterface::RTCOfferAnswerOptions& options);

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

private:
  friend class CreateSdpObserver;

private:
  uint64_t id_;
  WebrtcPeerConnectionObserver* webrtc_observer_;
  webrtc::PeerConnectionInterface::RTCConfiguration rtc_config_;
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc_;
};

}

#endif //LICODECPPCLIENT_WEBRTC_CONNECTION_H
