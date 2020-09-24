//
// Created by nbaiot@126.com on 2020/9/23.
//

#ifndef LICODECPPCLIENT_WEBSOCKET_SESSION_H
#define LICODECPPCLIENT_WEBSOCKET_SESSION_H

#include <memory>
#include <string>
#include <thread>
#include <cstdint>
#include <functional>

#include <boost/asio/io_context.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/core/tcp_stream.hpp>

namespace nbaiot {

class WebsocketSession : public std::enable_shared_from_this<WebsocketSession> {

public:
  explicit WebsocketSession(std::string url);

  ~WebsocketSession();

  void Connect();

  void Disconnect();

  void SendMsg(const std::string& msg);

private:
  bool ParseUrl();

  void OnResolve(
      boost::beast::error_code ec,
      const boost::asio::ip::tcp::resolver::results_type& results);

  void OnConnect(boost::beast::error_code ec,
                 const boost::asio::ip::tcp::resolver::results_type::endpoint_type& e);

  void OnHandshake(const boost::beast::error_code& ec);

private:
  std::string url_;
  std::string scheme_;
  std::string host_;
  int16_t port_;
  std::string target_;
  boost::asio::io_context ioc_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
  std::unique_ptr<std::thread> thread_;
  boost::asio::ip::tcp::resolver resolver_;
  boost::beast::websocket::stream<boost::beast::tcp_stream> ws_;
  boost::beast::flat_buffer buffer_;
};

}

#endif //LICODECPPCLIENT_WEBSOCKET_SESSION_H
