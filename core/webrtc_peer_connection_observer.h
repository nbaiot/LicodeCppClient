//
// Created by nbaiot@126.com on 2020/10/19.
//

#ifndef LICODECPPCLIENT_WEBRTC_PEER_CONNECTION_OBSERVER_H
#define LICODECPPCLIENT_WEBRTC_PEER_CONNECTION_OBSERVER_H

#include <string>
#include "api/peer_connection_interface.h"

namespace nbaiot {

class WebrtcConnection;

class WebrtcPeerConnectionObserver {
public:
  virtual ~WebrtcPeerConnectionObserver() = default;

  virtual void OnSdpCreateSuccess(WebrtcConnection* peer, webrtc::SdpType type, const std::string& sdp) = 0;

  virtual void OnSdpCreateFailure(WebrtcConnection* peer, const webrtc::RTCError& error) {}

  virtual void OnIceCandidate(WebrtcConnection* peer, const webrtc::IceCandidateInterface* candidate) = 0;

  virtual void OnPeerConnect(WebrtcConnection* peer) = 0;

  virtual void OnPeerDisconnect(WebrtcConnection* peer) = 0;

  virtual void OnAddRemoteVideoTrack(WebrtcConnection* peer, webrtc::VideoTrackInterface* video) {}

  virtual void OnRemoveRemoteVideoTrack(WebrtcConnection* peer, webrtc::VideoTrackInterface* video) {}

  virtual void OnAddRemoteAudioTrack(WebrtcConnection* peer, webrtc::AudioTrackInterface* audio) {}

  virtual void OnRemoveRemoteAudioTrack(WebrtcConnection* peer, webrtc::AudioTrackInterface* audio) {}

  virtual void OnDataChannel(WebrtcConnection* peer, webrtc::DataChannelInterface* dataChannel) {}

  /// TODO: support stats info callback

};

}

#endif //LICODECPPCLIENT_WEBRTC_PEER_CONNECTION_OBSERVER_H
