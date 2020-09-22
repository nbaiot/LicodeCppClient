//
// Created by nbaiot@126.com on 2020/9/23.
//

#ifndef LICODECPPCLIENT_WEBSOCKET_SESSION_H
#define LICODECPPCLIENT_WEBSOCKET_SESSION_H

#include <memory>
#include <string>
#include <cstdint>

#include <boost/asio/io_context.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/core/tcp_stream.hpp>

namespace nbaiot {

class WebsocketSession : public std::enable_shared_from_this<WebsocketSession> {

public:
  explicit WebsocketSession(boost::asio::io_context& ioc);

  ~WebsocketSession();

private:
  std::string host_;
  int16_t port_;
  boost::asio::ip::tcp::resolver resolver_;
  boost::beast::websocket::stream<boost::beast::tcp_stream> ws_;
  boost::beast::flat_buffer buffer_;
};

}

#endif //LICODECPPCLIENT_WEBSOCKET_SESSION_H
