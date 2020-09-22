//
// Created by nbaiot@126.com on 2020/9/23.
//

#ifndef LICODECPPCLIENT_HTTP_SESSION_H
#define LICODECPPCLIENT_HTTP_SESSION_H

#include <memory>
#include <string>
#include <cstdint>
#include <functional>

#include <boost/asio/io_context.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/core/tcp_stream.hpp>

namespace nbaiot {

class HttpSession : public std::enable_shared_from_this<HttpSession> {

public:
  using OnConnectCallback = std::function<void(bool success)>;

  explicit HttpSession(boost::asio::io_context& ioc);

  ~HttpSession();

  void Connect(const std::string& host, int16_t port, int32_t timeoutMS, OnConnectCallback callback);

  void SendData(const std::string& data);

private:
  void OnResolve(int32_t timeoutMS, OnConnectCallback callback,
                 boost::beast::error_code ec,
                 const boost::asio::ip::tcp::resolver::results_type& results);

  void OnConnect(OnConnectCallback callback,
                 boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type ep);

private:
  boost::asio::ip::tcp::resolver resolver_;
  boost::beast::tcp_stream stream_;
};

}


#endif //LICODECPPCLIENT_HTTP_SESSION_H
