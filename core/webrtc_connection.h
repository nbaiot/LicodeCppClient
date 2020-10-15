//
// Created by nbaiot@126.com on 2020/10/15.
//

#ifndef LICODECPPCLIENT_WEBRTC_CONNECTION_H
#define LICODECPPCLIENT_WEBRTC_CONNECTION_H

#include "api/peer_connection_interface.h"

namespace nbaiot {

class WebrtcConnection : public webrtc::PeerConnectionObserver,
                         public webrtc::CreateSessionDescriptionObserver {

public:

  explicit WebrtcConnection(const webrtc::PeerConnectionInterface::RTCConfiguration& config);


  /// sdp observer
  void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;

  void OnFailure(webrtc::RTCError error) override;

  /// pc observer
  void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;

  void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;

  void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;

  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;

private:
  webrtc::PeerConnectionInterface::RTCConfiguration rtc_config_;
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc_;
};


}

#endif //LICODECPPCLIENT_WEBRTC_CONNECTION_H
