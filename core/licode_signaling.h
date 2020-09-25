//
// Created by nbaiot@126.com on 2020/9/24.
//

#ifndef LICODECPPCLIENT_LICODE_SIGNALING_H
#define LICODECPPCLIENT_LICODE_SIGNALING_H

#include <memory>
#include <string>
#include <functional>

#include "licode_token.h"

namespace nbaiot {

class Worker;

class WebsocketSession;

class LicodeSignaling : public std::enable_shared_from_this<LicodeSignaling> {

public:
  using OnSignalingReadyCallback = std::function<void()>;

  using OnSignalingDisconnectCallback = std::function<void()>;

  explicit LicodeSignaling(std::shared_ptr<Worker> worker);

  enum State {
    kDisconnect,
    kConnecting,
    kConnected,
  };

  ~LicodeSignaling();

  bool Init(LicodeToken token);

  void Dispose();

  void SetOnSignalingReadyCallback(OnSignalingReadyCallback callback);

  State CurrentState();

private:
  void Token(bool singlePC = false);

  void UpdateState(State state);

  void OnWebsocketConnectCallback();

  void OnWebsocketMsg(const std::string& msg);

  void OnWebsocketDisconnect(const std::string& reason);

  void KeepAlive();

  bool SocketIoPing();

  void ReceivePong();

private:
  State state_;
  LicodeToken token_;
  std::string signaling_url_;
  std::string sid_;
  int32_t ping_interval_ms_;
  int32_t ping_timeout_ms_;
  std::shared_ptr<Worker> worker_;
  std::shared_ptr<WebsocketSession> websocket_;
  OnSignalingReadyCallback ready_callback_;
  OnSignalingDisconnectCallback disconnect_callback_;
};

}

#endif //LICODECPPCLIENT_LICODE_SIGNALING_H
