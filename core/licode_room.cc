//
// Created by nbaiot@126.com on 2020/10/2.
//

#include "licode_room.h"
#include "licode_signaling.h"
#include "licode_stream.h"
#include "licode_signaling_pkt_creator.h"
#include "webrtc_wrapper.h"
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
  signaling_->SetOnEventCallback([this](const std::string& event, const std::string& body) {
    OnEvent(event, body);
  });
  signaling_->SetOnSubscribeCallback([this](const std::string& msg) {
    OnSubscribeStream(msg);
  });
  signaling_->SetOnEventCallback([this](const std::string& event, const std::string& msg) {
    if (event == "connection_message_erizo") {
      OnErizoConnectionEvent(msg);
    } else {
      LOG(ERROR) << ">>>>>>>>>> receive unknown event:" << event << ",\n" << msg;
    }
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

void LicodeRoom::SubscribeStream(uint64_t streamId) {
  LOG(INFO) << ">>>>>>>>>>>>>>>> start subscribe stream id:" << streamId;
  auto pkt = LicodeSignalingPktCreator::CreateSubscribeStreamPkt(streamId);
  signaling_->SendMsg(signaling_->SubscribeStreamMsgHeader() + pkt);
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
      client_id_ = body["clientId"];
      auto streams = body["streams"];
      stream_infos_.clear();
      for (const auto& stream : streams) {
        auto info = std::make_shared<WebrtcStreamInfo>(
            stream["id"], stream["video"], stream["audio"], stream["data"],
            stream["label"], stream["screen"],
            /// TODO: fixme
            stream["attributes"].dump());
        stream_infos_.emplace(std::make_pair(stream["id"], info));
        /// TODO: fixme
        CreateSubscribePeerConnection(stream["id"]);
        SubscribeStream(stream["id"]);
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

    CreateSubscribePeerConnection(stream["id"]);
    SubscribeStream(stream["id"]);

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

void LicodeRoom::OnSubscribeStream(const std::string& msg) {
  /// erizoId 和 connectionId
  /// 发起 offer == > receive ErizoConnectionEvent:
  /// LicodeSignalingPktCreator::CreateConnectionOfferMsg()
  /// LicodeSignalingPktCreator::CreateConnectionPtk()

  /// 收到 answer 之后 发送 candidate
  /// TODO: 需要和 streamId 关联
  LOG(INFO) << ">>>>>>>>>>>>>>>>>OnSubscribeStream===" << msg;
  auto json = nlohmann::json::parse(msg);
  erizoId = json[1];
  connId = json[2];
}

void LicodeRoom::OnErizoConnectionEvent(const std::string& msg) {
  /// erizoJS/models/Connection.js
  /// 1. receive info:202, type CONN_SDP
  /// 2. receive info:104  type CONN_READY
  /// 3. receive info:150  type CONN_QUALITY_LEVEL

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
    default:
      LOG(INFO) << ">>>>> receive unknown erizo connection event";
  }

}

void LicodeRoom::PublishStream() {
  /// 首先更新 stream 属性
  /// ["onUpdateAttributeStream",{"id":903077823469367800,"attrs":{"type":"publisher"}}]
}

void LicodeRoom::receiveOffer(const std::string& connId, const std::string& sdp) {
  LOG(INFO) << ">>>>>>receive offer sdp:\n" << sdp;
  webrtc::SdpParseError error;
  auto sessionDescription = WebrtcWrapper::Instance()->CreateSessionDescription(webrtc::SdpType::kOffer, sdp, &error);
  if (!sessionDescription) {
    LOG(ERROR) << ">>>>>>>>>> CreateSessionDescription error:" << error.description;
    return;
  }
  auto it = peer_connections_.begin();
  it->second->SetRemoteDescription(std::move(sessionDescription));
  webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
  options.offer_to_receive_audio = 1;
  options.offer_to_receive_video = 1;
  it->second->CreateAnswer(options);
}

void LicodeRoom::receiveAnswer(const std::string& connId, const std::string& sdp) {

}

void LicodeRoom::CreateSubscribePeerConnection(uint64_t streamId) {
  if (peer_connections_[streamId]) {
    return;
  }
  webrtc::PeerConnectionInterface::RTCConfiguration config;
  config.sdp_semantics = webrtc::SdpSemantics::kPlanB;
  config.enable_dtls_srtp = true;
  webrtc::PeerConnectionInterface::IceServer server;
  server.uri = "stun:stun.l.google.com:19302";
  config.servers.push_back(server);
  peer_connections_[streamId] =
      rtc::scoped_refptr<rtc::RefCountedObject<WebrtcConnection>>(new rtc::RefCountedObject<WebrtcConnection>(config));

  /// TODO: fixme
  peer_connections_[streamId]->SetSdpCreateSuccessCallback([this, streamId](webrtc::SdpType type, const std::string& sdp){
    auto pkt = LicodeSignalingPktCreator::CreateOfferOrAnswerPkt(false, erizoId, connId, sdp, 300);
    LOG(INFO) << ">>>>>>>>>>>>>>>>>>0000000000000000";
    signaling_->SendMsg("422" + pkt);
  });
}

}