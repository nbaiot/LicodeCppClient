//
// Created by nbaiot@126.com on 2020/10/2.
//

#ifndef LICODECPPCLIENT_LICODE_ROOM_H
#define LICODECPPCLIENT_LICODE_ROOM_H

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

#include "licode_token.h"
#include "webrtc_stream_info.h"
#include "ice_server.h"

namespace nbaiot {

class Worker;

class LicodeSignaling;

class LicodeStream;

class LicodeRoom : public std::enable_shared_from_this<LicodeRoom> {

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

  void UnSubscriberStream();

  void LocalStreams();

  void RemoteStreams();

  void StartRecordingStream();

  void StopRecordingStream();

  void GetStreamsByAttribute();

private:
  void Update(State state);

  void OnJoinRoom(const std::string& msg);

  void OnLeaveRoom();

  void OnEvent(const std::string& event, const std::string& msg);

  void OnAddStream(const std::string& msg);

  void OnRemoveStream(const std::string& msg);

  void OnSubscribeStream(const std::string& msg);

  void OnErizoConnectionEvent(const std::string& msg);

private:
  LicodeToken token_;
  std::string id_;
  std::string client_id_;
  State state_;
  bool p2p_;
  bool single_pc_;
  int default_video_bw_;
  int max_video_bw_;
  std::shared_ptr<Worker> worker_;
  std::vector<IceServer> ice_server_list_;
  std::unordered_map<uint64_t, std::shared_ptr<WebrtcStreamInfo>> stream_infos_;
  std::unique_ptr<LicodeSignaling> signaling_;
  OnJoinRoomCallback join_room_callback_;
};

}

#endif //LICODECPPCLIENT_LICODE_ROOM_H
