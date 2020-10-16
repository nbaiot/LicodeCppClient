//
// Created by nbaiot@126.com on 2020/9/24.
//

#ifndef LICODECPPCLIENT_LICODE_SIGNALING_H
#define LICODECPPCLIENT_LICODE_SIGNALING_H

#include <memory>
#include <string>
#include <functional>

#include "licode_token.h"
#include "licode_stream_info.h"
#include "licode_event.h"

namespace nbaiot {

class Worker;

class WebsocketSession;

class LicodeSignaling : public std::enable_shared_from_this<LicodeSignaling> {

public:
  using OnInitTokenCallback = std::function<void(const std::string&)>;

  using OnDisconnectCallback = std::function<void()>;

  using OnEventCallback = std::function<void(const std::string& event, const std::string& msg)>;

  using OnSubscribeStreamCallback = std::function<void(const std::string&)>;

  explicit LicodeSignaling(std::shared_ptr<Worker> worker);

  enum State {
    kDisconnect,
    kConnecting,
    kConnected,
  };

  ~LicodeSignaling();

  bool Init(LicodeToken token);

  void Dispose();

  void SetOnInitTokenCallback(OnInitTokenCallback callback);

  void SetOnSignalingDisconnectCallback(OnDisconnectCallback callback);

  void SetOnEventCallback(OnEventCallback callback);

  void SetOnSubscribeCallback(OnSubscribeStreamCallback callback);

  State CurrentState();

  void SendMsg(const std::string& msg);

  std::string SubscribeStreamMsgHeader();

private:
  void InitToken(bool singlePC = false);

  void UpdateState(State state);

  void OnWebsocketConnectCallback();

  void OnWebsocketMsg(const std::string& msg);

  void OnWebsocketDisconnect(const std::string& reason);

  void KeepAlive();

  bool SocketIoPing();

  void ProcessSocketIOOpen(const std::string& msg);

  void ProcessInitTokenResponse(const std::string& msg);

  void ProcessDisconnect();

  void ProcessPong();

  void ProcessSubscribeStream(const std::string& msg);

  void ProcessEvent(const std::string& msg);

private:
  State state_;
  LicodeToken token_;
  std::string signaling_url_;
  std::string sid_;
  int32_t ping_interval_ms_;
  int32_t ping_timeout_ms_;
  std::shared_ptr<Worker> worker_;
  std::shared_ptr<WebsocketSession> websocket_;
  OnInitTokenCallback init_token_callback_;
  OnDisconnectCallback disconnect_callback_;
  OnEventCallback event_callback_;
  OnSubscribeStreamCallback subscribe_stream_callback_;
};

}

#endif //LICODECPPCLIENT_LICODE_SIGNALING_H
