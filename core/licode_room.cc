//
// Created by nbaiot@126.com on 2020/10/2.
//

#include "licode_room.h"
#include "licode_signaling.h"
#include "licode_signaling_pkt_builder.h"
#include "thread/worker.h"
#include "licode_room_observer.h"
#include <glog/logging.h>
#include <nlohmann/json.hpp>

namespace nbaiot {

LicodeRoom::LicodeRoom(std::shared_ptr<Worker> worker, LicodeToken token, LicodeRoomObserver* observer)
    : token_(std::move(token)), state_(kDisconnected), p2p_(false), single_pc_(false),
      default_video_bw_(0), max_video_bw_(0), worker_(std::move(worker)), observer_(observer) {
  BOOST_ASSERT(observer_);
  signaling_ = std::make_unique<LicodeSignaling>(worker_);
  signaling_->SetOnInitTokenCallback([this](const std::string& msg) {
    OnJoinRoom(msg);
  });
  signaling_->SetOnSignalingDisconnectCallback([this]() {
    OnLeaveRoom();
  });
  signaling_->SetOnEventCallback([this](const std::string& event, const std::string& body) {
    OnEvent(event, body);
  });
  signaling_->SetOnSubscribeCallback([this](const std::string& msg) {
    OnSubscribeStream(msg);
  });
  signaling_->SetOnPublishCallback([this](const std::string& msg) {
    OnPublishStream(msg);
  });
}


LicodeRoom::~LicodeRoom() = default;

std::string LicodeRoom::Id() {
  return id_;
}

LicodeRoom::State LicodeRoom::CurrentState() {
  return state_;
}

void LicodeRoom::Join() {
  if (CurrentState() != kDisconnected) {
    LOG(WARNING) << "already join room:" << id_;
    return;
  }
  Update(kConnecting);
  signaling_->Init(token_);
}

void LicodeRoom::Leave() {
  if (CurrentState() == kDisconnected) {
    LOG(WARNING) << "already leave room:" << id_;
    return;
  }
  signaling_->Dispose();
}

void LicodeRoom::PublishStream(const std::shared_ptr<LicodeStreamInfo>& info) {
  PublicStreamPktBuilder builder;
  /// TODO: fixme p2p
  auto pkt = builder.SetLabel(info->Label()).SetState("erizo").Build();

  worker_->PostTask(SafeTask([pkt, info](const std::shared_ptr<LicodeRoom>& room) {
    room->pending_publish_local_streams_.push(info);
    room->signaling_->SendMsg(LicodeSignaling::PublishStreamMsgHeader() + pkt);
  }));

}

void LicodeRoom::UnPublishStream(uint64_t streamId) {
  UnsubscribeStreamPktBuilder builder;
  auto pkt = builder.SetStreamId(streamId).Build();
  worker_->PostTask(SafeTask([pkt](const std::shared_ptr<LicodeRoom>& room) {
    room->signaling_->SendMsg(LicodeSignaling::EventHeader() + pkt);
  }));
}


void LicodeRoom::SubscribeStream(uint64_t streamId) {
  /// erizoClient/src/Room.js
  worker_->PostTask(SafeTask([streamId](const std::shared_ptr<LicodeRoom>& room) {
    LOG(INFO) << ">>>>>>>>>>>>>>>> start subscribe stream id:" << streamId;
    SubscribeStreamPktBuilder builder;
    auto pkt = builder.SetStreamId(streamId).SetOfferFromErizo(true).Build();
    room->pending_subscribe_streams_.push(streamId);
    room->signaling_->SendMsg(LicodeSignaling::SubscribeStreamMsgHeader() + pkt);
  }));
}

void LicodeRoom::UnSubscriberStream(uint64_t streamId) {
  worker_->PostTask(SafeTask([streamId](const std::shared_ptr<LicodeRoom>& room) {
    UnsubscribeStreamPktBuilder builder;
    auto pkt = builder.SetStreamId(streamId).Build();
    LOG(INFO) << ">>>>>>>>>>>>>>>> start unsubscribe stream id:" << pkt;
    room->signaling_->SendMsg(LicodeSignaling::EventHeader() + pkt);
  }));
}

void LicodeRoom::Update(LicodeRoom::State state) {
  state_ = state;
}

void LicodeRoom::OnJoinRoom(const std::string& msg) {

  try {
    auto json = nlohmann::json::parse(msg);
    if (json[0] == "success") {
      auto body = json[1];
      id_ = body["id"];
      client_id_ = body["clientId"];
      remote_stream_infos_.clear();
      std::vector<std::shared_ptr<LicodeStreamInfo>> existStreams;
      auto streams = body["streams"];
      for (const auto& stream : streams) {
        auto info = std::make_shared<LicodeStreamInfo>(
            stream["id"], stream["video"], stream["audio"], stream["data"],
            stream["label"], stream["screen"],
            stream["attributes"].dump());
        existStreams.push_back(info);
      }
      if (body.contains("p2p")) {
        p2p_ = body["p2p"];
      }
      default_video_bw_ = body["defaultVideoBW"];
      max_video_bw_ = body["maxVideoBW"];
      single_pc_ = body["singlePC"];

      auto iceServers = body["iceServers"];
      ice_server_list_.clear();
      ice_server_list_.reserve(iceServers.size());
      for (const auto& ice: iceServers) {
        IceServer iceServer;
        iceServer.url = ice.value<std::string>("url", "");
        iceServer.username = ice.value<std::string>("username", "");
        iceServer.credential = ice.value<std::string>("credential", "");
        ice_server_list_.push_back(iceServer);
      }
      Update(kConnected);

      worker_->PostTask(SafeTask([](const std::shared_ptr<LicodeRoom>& room) {
        room->observer_->OnJoinRoom(room->id_);
      }));

      for (const auto& info: existStreams) {
        worker_->PostTask(SafeTask([info](const std::shared_ptr<LicodeRoom>& room) {
          room->remote_stream_infos_.emplace(std::make_pair(info->Id(), info));
          room->observer_->OnAddRemoteStream(info);
        }));
      }
    } else {
      Update(kDisconnected);
      LOG(INFO) << ">>>>>>>>>> join room:" << id_ << " failed," << json[1];
    }
  } catch (const std::exception& e) {
    Update(kDisconnected);
    LOG(WARNING) << ">>>>>>>>>>OnJoinRoom error:" << e.what();
  }

}

void LicodeRoom::OnLeaveRoom() {
  Update(kDisconnected);
}


void LicodeRoom::OnEvent(const std::string& event, const std::string& msg) {
  if (event == "connection_message_erizo") {
    OnErizoConnectionEvent(msg);
  } else if (event == "onAddStream") {
    OnAddStream(msg);
  } else if (event == "onRemoveStream") {
    OnRemoveStream(msg);
  } else {
    LOG(ERROR) << ">>>>>>>>>> receive unknown event:" << event << ",\n" << msg;
  }
}

void LicodeRoom::OnAddStream(const std::string& msg) {
  try {
    auto stream = nlohmann::json::parse(msg);

    auto info = std::make_shared<LicodeStreamInfo>(
        stream["id"], stream["video"], stream["audio"], stream["data"],
        stream["label"], stream["screen"],
        /// TODO: fixme
        stream["attributes"].dump());
    worker_->PostTask(SafeTask([info](const std::shared_ptr<LicodeRoom>& room) {
      LOG(INFO) << "add stream:" << info->Id();
      auto localStreamInfo = room->local_stream_infos_[info->Id()];
      if (localStreamInfo) {
        info->SetConnectionId(localStreamInfo->ConnectionId());
        room->local_stream_infos_[info->Id()] = info;
      } else {
        room->remote_stream_infos_[info->Id()] = info;
        room->observer_->OnAddRemoteStream(info);
      }
    }));

  } catch (const std::exception& e) {
    LOG(WARNING) << ">>>>>>>>>>>OnAddStream error:" << e.what();
  }
}

void LicodeRoom::OnRemoveStream(const std::string& msg) {
  try {
    auto body = nlohmann::json::parse(msg);
    uint64_t removeStreamId = body["id"];
    worker_->PostTask(SafeTask([removeStreamId](const std::shared_ptr<LicodeRoom>& room) {
      LOG(INFO) << ">>>>>>>>>> remove stream:" << removeStreamId;
      auto it = room->remote_stream_infos_.find(removeStreamId);
      if (it != room->remote_stream_infos_.end()) {
        room->remote_stream_infos_.erase(it);
      }
    }));
  } catch (const std::exception& e) {
    LOG(WARNING) << ">>>>>>>>>>>OnRemoveStream error:" << e.what();
  }

}

void LicodeRoom::OnSubscribeStream(const std::string& msg) {
  /// receive erizoId and connectionId
  LOG(INFO) << ">>>>>>>>>>>>>>>>>OnSubscribeStream:\n" << msg;
  auto json = nlohmann::json::parse(msg);
  worker_->PostTask(SafeTask([json](const std::shared_ptr<LicodeRoom>& room) {
    auto streamId = room->pending_subscribe_streams_.front();
    room->pending_subscribe_streams_.pop();
    if (room->erizo_id_.empty()) {
      room->erizo_id_ = json[1];
    } else if (room->erizo_id_ != json[1]) {
      LOG(WARNING) << ">>>>>> stream " << streamId << " erizoId:" << json[1]
                   << ", but others erizoId:" << room->erizo_id_;
    }
    room->remote_stream_infos_[streamId]->SetConnectionId(json[2]);
    room->observer_->OnSubscribeStreamAllocateConnId(room->remote_stream_infos_[streamId]);
  }));

}

void LicodeRoom::OnPublishStream(const std::string& msg) {
  LOG(INFO) << ">>>>>>>>>>>>>>>>>OnPublishStream:\n" << msg;
  worker_->PostTask(SafeTask([msg](const std::shared_ptr<LicodeRoom>& room) {
    auto streamInfo = room->pending_publish_local_streams_.front();
    room->pending_publish_local_streams_.pop();
    auto json = nlohmann::json::parse(msg);
    if (room->erizo_id_.empty()) {
      room->erizo_id_ = json[1];
    } else if (room->erizo_id_ != json[1]) {
      LOG(WARNING) << ">>>>>> erizoId:" << json[1]
                   << ", but others erizoId:" << room->erizo_id_;
    }
    uint64_t streamId = json[0];
    streamInfo->SetId(streamId);
    streamInfo->SetConnectionId(json[2]);
    room->local_stream_infos_[streamId] = streamInfo;
  }));
}

void LicodeRoom::OnErizoConnectionEvent(const std::string& msg) {
  /// erizoJS/models/Connection.js
  /// 1. receive info:202, type CONN_SDP
  /// 2. receive info:104  type CONN_READY
  /// 3. receive info:150  type CONN_QUALITY_LEVEL
  /// 4. receive info:500  type failed

  auto event = nlohmann::json::parse(msg);
  std::string connectionId = event["connectionId"];
  int info = event["info"];
  auto evt = event["evt"];
  switch (info) {
    case 202:
      if (evt["type"] == "offer") {
        receiveOffer(connectionId, evt["sdp"]);
      } else if (evt["type"] == "answer") {
        receiveAnswer(connectionId, evt["sdp"]);
      }
      break;
    case 150:
      LOG(INFO) << ">>>>> receive quality_level";
      break;
    case 104:
      OnPeerConnectionConnectErizoReady(connectionId);
      break;
    case 500:
      LOG(INFO) << ">>>>> receive erizo connection error event";
      break;
    default:
      LOG(INFO) << ">>>>> receive unknown erizo connection event";
  }

}

void LicodeRoom::OnPeerConnectionConnectErizoReady(const std::string& connId) {
  worker_->PostTask(SafeTask([connId](const std::shared_ptr<LicodeRoom>& room) {
    bool isRemote = false;
    std::shared_ptr<LicodeStreamInfo> streamInfo;
    for (const auto& info: room->remote_stream_infos_) {
      if (info.second->ConnectionId() == connId) {
        info.second->SetConnected(true);
        isRemote = true;
        streamInfo = info.second;
        break;
      }
    }
    if (!isRemote) {
      for (const auto& info: room->local_stream_infos_) {
        if (info.second->ConnectionId() == connId) {
          info.second->SetConnected(true);
          streamInfo = info.second;
          break;
        }
      }
    }
    room->observer_->OnStreamConnectReady(streamInfo);
  }));
}

void LicodeRoom::receiveOffer(const std::string& connId, const std::string& sdp) {
  worker_->PostTask(SafeTask([connId, sdp](const std::shared_ptr<LicodeRoom>& room) {
    std::shared_ptr<LicodeStreamInfo> streamInfo;
    for (const auto& info: room->remote_stream_infos_) {
      if (info.second->ConnectionId() == connId) {
        streamInfo = info.second;
        break;
      }
    }
    room->observer_->OnReceiveOffer(streamInfo, sdp);
  }));

}

void LicodeRoom::receiveAnswer(const std::string& connId, const std::string& sdp) {

}

std::function<void()> LicodeRoom::SafeTask(const std::function<void(std::shared_ptr<LicodeRoom>)>& function) {
  auto weakSelf = weak_from_this();
  return [weakSelf, function]() {
    auto self = weakSelf.lock();
    if (self) {
      function(self);
    }
  };
}


void LicodeRoom::SendAnswer(uint64_t streamId, const std::string& sdp) {

  worker_->PostTask(
      SafeTask(
          [streamId, sdp](const std::shared_ptr<LicodeRoom>& room) {
            OfferOrAnswerPktBuilder builder;
            builder.SetType("answer")
                .SetConnId(room->remote_stream_infos_[streamId]->ConnectionId())
                .SetErizoId(room->erizo_id_)
                .SetSdp(sdp);
            auto pkt = builder.Build();
            room->signaling_->SendMsg(LicodeSignaling::EventHeader() + pkt);
          }));
}

void LicodeRoom::SendOffer(uint64_t streamId, const std::string& sdp) {

  worker_->PostTask(
      SafeTask(
          [streamId, sdp](const std::shared_ptr<LicodeRoom>& room) {

            OfferOrAnswerPktBuilder builder;
            builder.SetType("offer")
                .SetConnId(room->remote_stream_infos_[streamId]->ConnectionId())
                .SetErizoId(room->erizo_id_)
                .SetSdp(sdp);
            auto pkt = builder.Build();
            room->signaling_->SendMsg(LicodeSignaling::EventHeader() + pkt);
          }));
}

void LicodeRoom::SendIceCandidate(uint64_t streamId,
                                  const std::string& sdpMid,
                                  int sdpMLineIndex,
                                  const std::string& candidate) {
  worker_->PostTask(
      SafeTask([streamId, sdpMid, sdpMLineIndex, candidate](const std::shared_ptr<LicodeRoom>& room) {
        ConnectionPtkBuilder builder;
        auto info = room->remote_stream_infos_[streamId];
        if (!info)
          info = room->local_stream_infos_[streamId];
        builder.SetErizoId(room->erizo_id_)
            .SetConnId(info->ConnectionId())
            .SetSdpMid(sdpMid)
            .SetSdpMLineIndex(sdpMLineIndex)
            .SetCandidate_(candidate);
        auto pkt = builder.Build();
        room->signaling_->SendMsg(LicodeSignaling::EventHeader() + pkt);
      }));
}

std::vector<IceServer> LicodeRoom::IceServers() {
  return ice_server_list_;
}

bool LicodeRoom::P2p() {
  return p2p_;
}

bool LicodeRoom::SinglePC() {
  return single_pc_;
}

int LicodeRoom::MaxVideoBW() {
  return max_video_bw_;
}

int LicodeRoom::DefaultVideoBW() {
  return default_video_bw_;
}

std::string LicodeRoom::RoomId() {
  return id_;
}

}