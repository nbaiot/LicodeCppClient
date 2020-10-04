//
// Created by nbaiot@126.com on 2020/10/2.
//

#include "licode_room.h"
#include "licode_signaling.h"
#include "licode_stream.h"
#include <glog/logging.h>
#include <nlohmann/json.hpp>

namespace nbaiot {

LicodeRoom::LicodeRoom(std::shared_ptr<Worker> worker, LicodeToken token)
    : token_(std::move(token)), state_(kDisconnected), p2p_(false), single_pc_(false),
      default_video_bw_(0), max_video_bw_(0), worker_(std::move(worker)) {
  signaling_ = std::make_unique<LicodeSignaling>(worker_);
  signaling_->SetOnInitTokenCallback([this](const std::string& msg) {
    OnJoinRoom(msg);
  });
  signaling_->SetOnSignalingDisconnectCallback([this]() {
    OnLeaveRoom();
  });
  signaling_->SetOnEventCallback([this](const std::string& event, const std::string& body){
    OnEvent(event, body);
  });
}


LicodeRoom::~LicodeRoom() {

}

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

void LicodeRoom::Update(LicodeRoom::State state) {
  state_ = state;
}

void LicodeRoom::OnJoinRoom(const std::string& msg) {

  try {
    auto json = nlohmann::json::parse(msg);
    Update(kConnected);
    if (json[0] == "success") {
      auto body = json[1];
      LOG(INFO) << ">>>>>>>>>> join room:" << id_ << " success";
      id_ = body["id"];

      auto streams = body["streams"];
      stream_infos_.clear();
      for (const auto& stream : streams) {
        auto info = std::make_shared<WebrtcStreamInfo>(
            stream["id"], stream["video"], stream["audio"], stream["data"],
            stream["label"], stream["screen"],
            /// TODO: fixme
            stream["attributes"].dump());
        stream_infos_.emplace(std::make_pair(stream["id"], info));
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
      if (join_room_callback_) {
        join_room_callback_();
      }

      /// TODO: stream
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
  if (event == "onRemoveStream") {
    OnRemoveStream(msg);
  } else if (event == "onAddStream") {
    OnAddStream(msg);
  }
}

void LicodeRoom::OnAddStream(const std::string& msg) {
  try {
    auto stream = nlohmann::json::parse(msg);
    auto info = std::make_shared<WebrtcStreamInfo>(
        stream["id"], stream["video"], stream["audio"], stream["data"],
        stream["label"], stream["screen"],
        /// TODO: fixme
        stream["attributes"].dump());
    stream_infos_[stream["id"]] = info;
    LOG(INFO) << "add stream:" << stream["id"];
  } catch (const std::exception& e) {
    LOG(WARNING) << ">>>>>>>>>>>OnAddStream error:" << e.what();
  }
}

void LicodeRoom::OnRemoveStream(const std::string& msg) {
  try {
    auto body = nlohmann::json::parse(msg);
    uint64_t removeStreamId = body["id"];
    auto it = stream_infos_.find(removeStreamId);
    if (it != stream_infos_.end()) {
      stream_infos_.erase(it);
    }
    LOG(INFO) << ">>>>>>>>>> remove stream:" << removeStreamId;
  } catch (const std::exception& e) {
    LOG(WARNING) << ">>>>>>>>>>>OnRemoveStream error:" << e.what();
  }

}

}