//
// Created by nbaiot@126.com on 2020/9/23.
//

#include "http_session.h"

#include <glog/logging.h>
#include <boost/asio/strand.hpp>
#include <boost/asio/connect.hpp>

namespace nbaiot {

HttpSession::HttpSession(boost::asio::io_context& ioc)
    : resolver_(boost::asio::make_strand(ioc)), stream_(boost::asio::make_strand(ioc)) {

}

HttpSession::~HttpSession() {

}

void HttpSession::Connect(const std::string& host, int16_t port, int32_t timeoutMS,
                          HttpSession::OnConnectCallback callback) {


  try {
    resolver_.async_resolve(host, std::to_string(port),
                            boost::beast::bind_front_handler(&HttpSession::OnResolve, shared_from_this(),
                                                             timeoutMS, std::move(callback)));
  } catch (const boost::system::system_error& error) {
    LOG(ERROR) << ">>>>>>>>> HttpSession resolve " << host << ":" << port
               << " failed, error:" << error.what();
    return;
  }

}

void HttpSession::SendData(const std::string& data) {

}

void HttpSession::OnResolve(int32_t timeoutMS, OnConnectCallback callback,
                            boost::beast::error_code ec, const boost::asio::ip::tcp::resolver::results_type& results) {

  if (ec) {
    LOG(ERROR) << ">>>>>HttpSession resolve error:" << ec.message();
    return;
  }

  /// Set the timeout for the operation
  stream_.expires_after(std::chrono::seconds(timeoutMS));

  /// Make the connection on the IP address we get from a lookup
  stream_.async_connect(
      results,
      boost::beast::bind_front_handler(
          &HttpSession::OnConnect,
          shared_from_this(), std::move(callback)));
}

void
HttpSession::OnConnect(OnConnectCallback callback, boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type ep) {
  if (ec) {
    LOG(ERROR) << ">>>>>HttpSession OnConnect error:" << ec.message();
    if (callback) {
      callback(false);
    }
    return;
  }
  if (callback) {
    callback(true);
  }
}

}