//
// Created by nbaiot@126.com on 2020/9/24.
//

#include "licode_signaling.h"

#include <glog/logging.h>

#include <nlohmann/json.hpp>
#include "thread/worker.h"
#include "messenger/websocket_session.h"
#include "licode_signaling_pkt_parser.h"
#include "licode_signaling_pkt_creator.h"

namespace nbaiot {

static const std::string SOCKET_IO_OPEN = "0";
static const std::string SOCKET_IO_CLOSE = "1";
static const std::string SOCKET_IO_PING = "2";
static const std::string SOCKET_IO_PONG = "3";
static const std::string SOCKET_IO_MESSAGE = "4";
static const std::string SOCKET_IO_NOOP = "6";

static const std::string SOCKET_IO_PACKET_CONNECT = "0";
static const std::string SOCKET_IO_PACKET_DISCONNECT = "1";
static const std::string SOCKET_IO_PACKET_EVENT = "2";
static const std::string SOCKET_IO_PACKET_ACK = "3";
static const std::string SOCKET_IO_PACKET_ERROR = "4";
static const std::string SOCKET_IO_PACKET_BINARY_EVENT = "5";
static const std::string SOCKET_IO_PACKET_BINARY_ACK = "6";


static const std::string SOCKET_IO_WEBSOCKET_CONNECT = SOCKET_IO_MESSAGE + SOCKET_IO_PACKET_CONNECT;
static const std::string SOCKET_IO_WEBSOCKET_DISCONNECT = SOCKET_IO_MESSAGE + SOCKET_IO_PACKET_DISCONNECT;
static const std::string SOCKET_IO_WEBSOCKET_EVENT = SOCKET_IO_MESSAGE + SOCKET_IO_PACKET_EVENT;

static const int64_t INIT_TOKEN_TRANS_ID = 111;
static const std::string TOKEN_INIT_RESPONSE =
    SOCKET_IO_MESSAGE + SOCKET_IO_PACKET_ACK + std::to_string(INIT_TOKEN_TRANS_ID);

LicodeSignaling::LicodeSignaling(std::shared_ptr<Worker> worker)
    : state_(kDisconnect), ping_interval_ms_(25000),
      ping_timeout_ms_(60000),
      worker_(std::move(worker)) {

}

LicodeSignaling::~LicodeSignaling() {
  websocket_->SetOnReadyCallback(nullptr);
  websocket_->SetOnReceiveMsgCallback(nullptr);
  websocket_->SetOnAbnormalDisconnectCallback(nullptr);
}

bool LicodeSignaling::Init(LicodeToken token) {
  if (CurrentState() != kDisconnect)
    return false;
  token_ = std::move(token);
  signaling_url_ = "ws://" + token_.host + "/socket.io/?EIO=3&transport=websocket";
  websocket_ = std::make_shared<WebsocketSession>(signaling_url_);
  websocket_->SetOnReadyCallback([this]() {
      OnWebsocketConnectCallback();
  });
  websocket_->SetOnReceiveMsgCallback([this](const std::string& msg) {
      OnWebsocketMsg(msg);
  });
  websocket_->SetOnAbnormalDisconnectCallback([this](const std::string& reason) {
      OnWebsocketDisconnect(reason);
  });

  UpdateState(kConnecting);
  websocket_->Connect();
  return true;
}

void LicodeSignaling::Dispose() {
  if (websocket_->IsConnected()) {
    websocket_->Disconnect();
  }
}

void LicodeSignaling::SetOnInitTokenCallback(LicodeSignaling::OnInitTokenCallback callback) {
  init_token_callback_ = std::move(callback);
}

void LicodeSignaling::SetOnSignalingDisconnectCallback(LicodeSignaling::OnDisconnectCallback callback) {
  disconnect_callback_ = std::move(callback);
}

void LicodeSignaling::SetOnEventCallback(LicodeSignaling::OnEventCallback callback) {
  event_callback_ = std::move(callback);
}

void LicodeSignaling::UpdateState(LicodeSignaling::State state) {
  state_ = state;
}

LicodeSignaling::State LicodeSignaling::CurrentState() {
  return state_;
}

void LicodeSignaling::OnWebsocketConnectCallback() {
  LOG(INFO) << ">>>>>>>>>> LicodeSignaling connect";
}

void LicodeSignaling::OnWebsocketMsg(const std::string& msg) {
  LOG(INFO) << ">>>>>>>>>> LicodeSignaling receive:" << msg;
  if (msg.find(SOCKET_IO_OPEN) == 0) {
    ProcessSocketIOOpen(msg.substr(SOCKET_IO_OPEN.length()));
  } else if (msg == SOCKET_IO_PONG) {
    ProcessPong();
  } else if (msg == SOCKET_IO_WEBSOCKET_CONNECT) {
    UpdateState(kConnected);
    KeepAlive();
    InitToken();
  } else if (msg.find(TOKEN_INIT_RESPONSE) == 0) {
    ProcessInitTokenResponse(msg.substr(TOKEN_INIT_RESPONSE.length()));
  } else if (msg == SOCKET_IO_WEBSOCKET_DISCONNECT) {
    ProcessDisconnect();
  } else if (msg.find(SOCKET_IO_WEBSOCKET_EVENT) == 0) {
    ProcessEvent(msg.substr(SOCKET_IO_WEBSOCKET_EVENT.length()));
  }
}

void LicodeSignaling::OnWebsocketDisconnect(const std::string& reason) {
  LOG(INFO) << ">>>>>>>>>> LicodeSignaling disconnect:" << reason;
  UpdateState(kDisconnect);
}

void LicodeSignaling::InitToken(bool singlePC) {
  std::string sendPkt =
      std::move(LicodeSignalingPktCreator::CreateTokenPkt(singlePC, token_));

  websocket_->SendMsg(SOCKET_IO_MESSAGE +
                      SOCKET_IO_PACKET_EVENT +
                      std::to_string(INIT_TOKEN_TRANS_ID) +
                      sendPkt);
}

void LicodeSignaling::KeepAlive() {
  worker_->ScheduleEvery([this]() -> bool {
      LOG(INFO) << ">>>>>>>>>> send ping";
      return SocketIoPing();
  }, std::chrono::milliseconds(ping_interval_ms_));
}

bool LicodeSignaling::SocketIoPing() {
  /// socket.io 中 websocket 有一套自己的ping/pong机制
  /// 使用的是 opcode 为 0x1(即Text) 类型的数据帧，
  /// 不是 websocket 协议规定的 ping(0x9)/pong(0xa) 类型的帧
  if (CurrentState() == kConnected) {
    websocket_->SendMsg(SOCKET_IO_PING);
    return true;
  }
  return false;
}

void LicodeSignaling::ProcessInitTokenResponse(const std::string& msg) {
  if (init_token_callback_) {
    init_token_callback_(msg);
  }
}

void LicodeSignaling::ProcessSocketIOOpen(const std::string& msg) {
  LicodeSignalingPktParser::ParseOpenMsg(msg,sid_, ping_interval_ms_, ping_timeout_ms_);
}

void LicodeSignaling::ProcessDisconnect() {
  bool needCallback = CurrentState() == kConnected;
  UpdateState(kDisconnect);
  Dispose();
  if (needCallback) {
    disconnect_callback_();
  }
}

void LicodeSignaling::ProcessPong() {
  LOG(INFO) << ">>>>>>>>>> receive pong";
  /// TODO: check timeout: ping_interval_ms_ + ping_timeout_ms_
}

void LicodeSignaling::ProcessEvent(const std::string& msg) {
  try {
    auto event = nlohmann::json::parse(msg);
    if (event_callback_) {
      event_callback_(event[0], event[1].dump());
    }
  } catch (const std::exception& e) {
    LOG(WARNING) << ">>>>>>>>>> ProcessEvent error:" << e.what();
  }

}

}
