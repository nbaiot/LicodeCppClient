//
// Created by nbaiot@126.com on 2020/10/2.
//

#ifndef LICODECPPCLIENT_LICODE_ROOM_H
#define LICODECPPCLIENT_LICODE_ROOM_H

#include <queue>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>
#include <nlohmann/json.hpp>

#include "licode_token.h"
#include "licode_stream_info.h"
#include "ice_server.h"
#include "webrtc_connection.h"


namespace nbaiot {

class Worker;

class LicodeSignaling;

class LicodeRoom : public std::enable_shared_from_this<LicodeRoom>,
                   public WebrtcPeerConnectionObserver {

public:
  enum State {
    kDisconnected = 0,
    kConnecting,
    kConnected,
  };

  using OnJoinRoomCallback = std::function<void()>;

  explicit LicodeRoom(std::shared_ptr<Worker> worker, LicodeToken token);

  ~LicodeRoom();

  std::string Id();

  State CurrentState();

  void Join();

  void Leave();

  void PublishStream();

  void UnPublishStream();

  /// TODO: audio, video, data
  void SubscribeStream(uint64_t streamId);

  void UnSubscriberStream(uint64_t streamId);

  void LocalStreams();

  void RemoteStreams();

  void StartRecordingStream();

  void StopRecordingStream();

  void GetStreamsByAttribute();

  /// webrtc connection callback
  void OnSdpCreateSuccess(WebrtcConnection* peer, webrtc::SdpType type, const std::string& sdp) override;

  void OnIceCandidate(WebrtcConnection* peer, const webrtc::IceCandidateInterface* candidate) override;

  void OnPeerConnect(WebrtcConnection* peer) override;

  void OnPeerDisconnect(WebrtcConnection* peer) override;

  void OnAddRemoteVideoTrack(WebrtcConnection* peer, webrtc::VideoTrackInterface* video) override;

  void OnRemoveRemoteVideoTrack(WebrtcConnection* peer, webrtc::VideoTrackInterface* video) override;

  void OnAddRemoteAudioTrack(WebrtcConnection* peer, webrtc::AudioTrackInterface* audio) override;

  void OnRemoveRemoteAudioTrack(WebrtcConnection* peer, webrtc::AudioTrackInterface* audio) override;

  void OnDataChannel(WebrtcConnection* peer, webrtc::DataChannelInterface* dataChannel) override;

private:
  void Update(State state);

  void OnJoinRoom(const std::string& msg);

  void OnLeaveRoom();

  void OnEvent(const std::string& event, const std::string& msg);

  void OnAddStream(const std::string& msg);

  void OnRemoveStream(const std::string& msg);

  void OnSubscribeStream(const std::string& msg);

  void OnErizoConnectionEvent(const std::string& msg);

  void OnPeerConnectionConnectErizoReady(const std::string& connId);

  void receiveOffer(const std::string& connId, const std::string& sdp);

  void receiveAnswer(const std::string& connId, const std::string& sdp);

  void CreateSubscribePeerConnection(uint64_t streamId);

  std::function<void()> SafeTask(const std::function<void(std::shared_ptr<LicodeRoom>)>& function);

private:
  LicodeToken token_;
  std::string id_;
  std::string client_id_;
  std::string erizo_id_;
  State state_;
  bool p2p_;
  bool single_pc_;
  int default_video_bw_;
  int max_video_bw_;
  std::shared_ptr<Worker> worker_;
  std::vector<IceServer> ice_server_list_;

  /// must be operation at worker thread
  std::unordered_map<uint64_t, std::shared_ptr<LicodeStreamInfo>> local_stream_infos_;
  std::unordered_map<uint64_t, std::shared_ptr<LicodeStreamInfo>> remote_stream_infos_;
  std::unordered_map<uint64_t, std::unique_ptr<WebrtcConnection>> peer_connections_;
  std::queue<uint64_t> pending_subscribe_streams_;


  std::unique_ptr<LicodeSignaling> signaling_;
  OnJoinRoomCallback join_room_callback_;

};

}

#endif //LICODECPPCLIENT_LICODE_ROOM_H
