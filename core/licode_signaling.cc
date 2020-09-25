//
// Created by nbaiot@126.com on 2020/9/24.
//

#include "licode_signaling.h"

#include <glog/logging.h>

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

void LicodeSignaling::OnWebsocketConnectCallback() {
  LOG(INFO) << ">>>>>>>>>> LicodeSignaling connect";
}

void LicodeSignaling::OnWebsocketMsg(const std::string& msg) {
  LOG(INFO) << ">>>>>>>>>> LicodeSignaling receive:" << msg;
  if (msg.find_first_of(SOCKET_IO_OPEN) == 0) {
    LicodeSignalingPktParser::ParseOpenMsg(msg.substr(SOCKET_IO_OPEN.length()),
                                           sid_, ping_interval_ms_, ping_timeout_ms_);
  } else if (msg == SOCKET_IO_PONG) {

  } else if (msg == SOCKET_IO_WEBSOCKET_CONNECT) {
    UpdateState(kConnected);
    KeepAlive();
    Token();
    /// TODO: token success == ready
//    if (ready_callback_) {
//      ready_callback_();
//    }
  } else if (msg == SOCKET_IO_WEBSOCKET_DISCONNECT) {
    UpdateState(kDisconnect);
    Dispose();
  }
}

void LicodeSignaling::OnWebsocketDisconnect(const std::string& reason) {
  LOG(INFO) << ">>>>>>>>>> LicodeSignaling disconnect:" << reason;
  UpdateState(kDisconnect);
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

void LicodeSignaling::KeepAlive() {
  worker_->ScheduleEvery([this]() -> bool {
      LOG(INFO) << ">>>>>>>>>> ping";
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

void LicodeSignaling::ReceivePong() {
  /// TODO: check timeout: ping_interval_ms_ + ping_timeout_ms_
}

void LicodeSignaling::Token(bool singlePC) {
  std::string sendPkt =
      std::move(LicodeSignalingPktCreator::CreateTokenPkt(singlePC, token_));

  websocket_->SendMsg(SOCKET_IO_MESSAGE +
                      SOCKET_IO_PACKET_EVENT +
                      std::to_string(111) +
                      sendPkt);
}

void LicodeSignaling::SetOnSignalingReadyCallback(LicodeSignaling::OnSignalingReadyCallback callback) {
  ready_callback_ = std::move(callback);
}

void LicodeSignaling::UpdateState(LicodeSignaling::State state) {
  state_ = state;
}

LicodeSignaling::State LicodeSignaling::CurrentState() {
  return state_;
}

}
