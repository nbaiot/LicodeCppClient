//
// Created by nbaiot@126.com on 2020/9/23.
//

#ifndef LICODECPPCLIENT_WEBSOCKET_SESSION_H
#define LICODECPPCLIENT_WEBSOCKET_SESSION_H

#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <cstdint>
#include <functional>

#include <boost/asio/io_context.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/core/tcp_stream.hpp>

namespace nbaiot {

class WebsocketSession : public std::enable_shared_from_this<WebsocketSession> {

public:
  using OnReadyCallback = std::function<void()>;

  using OnReceiveMsgCallback = std::function<void(const std::string& msg)>;

  using OnAbnormalDisconnectCallback = std::function<void(const std::string& reason)>;

  explicit WebsocketSession(std::string url);

  ~WebsocketSession();

  void Connect();

  void Disconnect();

  bool IsConnected();

  void SendMsg(const std::string& msg);

  void SetOnReadyCallback(OnReadyCallback callback);

  void SetOnReceiveMsgCallback(OnReceiveMsgCallback callback);

  void SetOnAbnormalDisconnectCallback(OnAbnormalDisconnectCallback callback);

private:
  bool ParseUrl();

  void OnResolve(
      boost::beast::error_code ec,
      const boost::asio::ip::tcp::resolver::results_type& results);

  void OnConnect(boost::beast::error_code ec,
                 const boost::asio::ip::tcp::resolver::results_type::endpoint_type& e);

  void OnHandshake(const boost::beast::error_code& ec);

  void DoRead();

  void OnRead(const boost::beast::error_code& ec,
              std::size_t bytes_transferred);

  void OnSend(const std::shared_ptr<std::string const>& msg);

  void OnWrite(const boost::beast::error_code& ec, std::size_t);

  void DoClose();

  void ProcessDisconnect(const std::string& reason);

private:
  std::string url_;
  /// TODO: support wss
  std::string scheme_;
  std::string host_;
  int16_t port_;
  std::string target_;
  bool do_disconnect_;
  std::atomic<bool> connected_;
  boost::asio::io_context ioc_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
  std::unique_ptr<std::thread> thread_;
  boost::asio::ip::tcp::resolver resolver_;
  boost::beast::websocket::stream<boost::beast::tcp_stream> ws_;
  boost::beast::flat_buffer buffer_;
  std::vector<std::shared_ptr<std::string const>> queue_;
  OnReadyCallback ready_callback_;
  OnReceiveMsgCallback receive_msg_callback_;
  OnAbnormalDisconnectCallback abnormal_disconnect_callback_;
};

}

#endif //LICODECPPCLIENT_WEBSOCKET_SESSION_H
