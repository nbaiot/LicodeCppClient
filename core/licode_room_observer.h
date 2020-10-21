//
// Created by nbaiot@126.com on 2020/10/20.
//

#ifndef LICODECPPCLIENT_LICODE_ROOM_OBSERVER_H
#define LICODECPPCLIENT_LICODE_ROOM_OBSERVER_H

#include <memory>
#include <string>

namespace nbaiot {

class LicodeRoomInfo;

class LicodeRoomObserver {

public:
  virtual ~LicodeRoomObserver() = default;

  virtual void OnJoinRoom(const std::string& roomId) = 0;

  virtual void OnLeaveRoom(const std::string& roomId) = 0;

  virtual void OnAddRemoteStream(const std::shared_ptr<LicodeStreamInfo>& info) = 0;

  virtual void OnRemoveRemoteStream(const std::shared_ptr<LicodeStreamInfo>& info) = 0;

  virtual void OnSubscribeStreamAllocateConnId(const std::shared_ptr<LicodeStreamInfo>& info) = 0;

  virtual void OnPublishStreamAllocateConnId(const std::shared_ptr<LicodeStreamInfo>& info) = 0;

  virtual void OnStreamConnectReady(const std::shared_ptr<LicodeStreamInfo>& info) = 0;

  virtual void OnReceiveOffer(const std::shared_ptr<LicodeStreamInfo>& info, const std::string& sdp) = 0;

  virtual void OnReceiveAnswer(const std::shared_ptr<LicodeStreamInfo>& info, const std::string& sdp) = 0;
};

}

#endif //LICODECPPCLIENT_LICODE_ROOM_OBSERVER_H
