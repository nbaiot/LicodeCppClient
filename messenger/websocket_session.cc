//
// Created by nbaiot@126.com on 2020/9/23.
//

#include "websocket_session.h"

#include <glog/logging.h>
#include <boost/asio/post.hpp>
#include <boost/beast/core/buffers_to_string.hpp>

namespace nbaiot {

WebsocketSession::WebsocketSession(std::string url)
    : url_(std::move(url)), port_(-1), do_disconnect_(false), connected_(false),
      work_guard_(ioc_.get_executor()), resolver_(ioc_), ws_(ioc_) {
  thread_ = std::make_unique<std::thread>([this]() {
      ioc_.run();
  });

  if (!ParseUrl()) {
    LOG(ERROR) << ">>>>>>>>> parse url:" << url_ << " failed";
  }
}


WebsocketSession::~WebsocketSession() {
  if (thread_ && thread_->joinable()) {
    ioc_.stop();
    thread_->join();
  }

  if (ws_.is_open()) {
    ws_.close("close websocket");
  }
}

void WebsocketSession::Connect() {
  if (connected_)
    return;
  do_disconnect_ = false;
  connected_ = false;
  resolver_.async_resolve(host_, std::to_string(port_),
                          boost::beast::bind_front_handler(&WebsocketSession::OnResolve, shared_from_this()));
}

void WebsocketSession::Disconnect() {
  do_disconnect_ = true;
  boost::asio::post(ws_.get_executor(),
                    boost::beast::bind_front_handler(&WebsocketSession::DoClose, shared_from_this()));
}

void WebsocketSession::SendMsg(const std::string& msg) {
  if (!IsConnected())
    return;
  auto ss = std::make_shared<std::string const>(msg);
  boost::asio::post(
      ws_.get_executor(),
      boost::beast::bind_front_handler(
          &WebsocketSession::OnSend,
          shared_from_this(),
          ss));
}

void WebsocketSession::OnResolve(boost::beast::error_code ec,
                                 const boost::asio::ip::tcp::resolver::results_type& results) {
  if (ec) {
    ProcessDisconnect(ec.message());
    return;
  }
  boost::beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(5));
  boost::beast::get_lowest_layer(ws_).async_connect(
      results,
      boost::beast::bind_front_handler(
          &WebsocketSession::OnConnect,
          shared_from_this()));

}

void WebsocketSession::OnConnect(boost::beast::error_code ec,
                                 const boost::asio::ip::tcp::resolver::results_type::endpoint_type& e) {
  if (ec) {
    ProcessDisconnect(ec.message());
    return;
  }

  boost::beast::get_lowest_layer(ws_).expires_never();

  ws_.set_option(
      boost::beast::websocket::stream_base::timeout::suggested(
          boost::beast::role_type::client));

  ws_.set_option(boost::beast::websocket::stream_base::decorator(
      [](boost::beast::websocket::request_type& req) {
          req.set(boost::beast::http::field::user_agent,
                  std::string(BOOST_BEAST_VERSION_STRING) +
                  " websocket-client-async");
      }));

  auto host = host_ + ":" + std::to_string(port_);
  ws_.async_handshake(host, target_,
                      boost::beast::bind_front_handler(
                          &WebsocketSession::OnHandshake,
                          shared_from_this()));
}

void WebsocketSession::OnHandshake(const boost::beast::error_code& ec) {
  if (ec) {
    ProcessDisconnect(ec.message());
    return;
  }

  connected_ = true;

  if (ready_callback_) {
    ready_callback_();
  }

  DoRead();
}

void WebsocketSession::DoRead() {
  ws_.async_read(
      buffer_,
      boost::beast::bind_front_handler(
          &WebsocketSession::OnRead,
          shared_from_this()));
}


void WebsocketSession::OnRead(const boost::beast::error_code& ec, std::size_t bytes_transferred) {
  if (ec) {
    ProcessDisconnect(ec.message());
    return;
  }

  auto msg = boost::beast::buffers_to_string(buffer_.data());
  if (receive_msg_callback_) {
    receive_msg_callback_(msg);
  }
  buffer_.consume(buffer_.size());

  DoRead();
}

bool WebsocketSession::ParseUrl() {
  if (url_.empty() || url_.size() <= 5) {
    return false;
  }
  /// scheme
  auto scheme = url_.substr(0, url_.find_first_of(':'));
  if (scheme.empty()) {
    LOG(ERROR) << "unexpected url:" << url_;
    return false;
  }
  std::transform(scheme.begin(), scheme.end(), scheme.begin(), ::tolower);
  if (scheme == "ws") {
    scheme_ = "ws";
  } else if (scheme == "wss") {
    scheme_ = "wss";
  } else {
    LOG(ERROR) << "unexpected url:" << url_;
    return false;
  }

  /// host + (target + params)
  auto pathStart = scheme_.size() + 3;
  auto pathEnd = url_.size();
  auto hostPath = url_.substr(pathStart, pathEnd - pathStart);

  if (hostPath.empty()) {
    LOG(ERROR) << "unexpected url:" << url_;
    return false;
  }

  /// host
  auto portStart = hostPath.find_first_of(':');
  pathStart = hostPath.find_first_of('/');

  if (portStart == std::string::npos) {
    /// default port
    port_ = 8080;
    if (pathStart == std::string::npos) {
      host_ = hostPath;
    } else {
      host_ = hostPath.substr(0, pathStart);
    }
  } else {
    host_ = hostPath.substr(0, portStart);
    auto portEnd = hostPath.find_first_of('/');
    std::string portS;
    if (portEnd == std::string::npos) {
      portS = hostPath.substr(portStart + 1);
    } else {
      portS = hostPath.substr(portStart + 1, portEnd - (portStart + 1));
    }
    if (!portS.empty()) {
      /// TODO: fixme, not number?
      port_ = std::atoi(portS.c_str());
    } else {
      port_ = 8080;
    }
  }
  /// target
  target_ = hostPath.substr(pathStart + 1);
  target_ = "/" + target_;
  return true;
}

void WebsocketSession::SetOnReadyCallback(WebsocketSession::OnReadyCallback callback) {
  connected_ = true;
  ready_callback_ = std::move(callback);
}

void WebsocketSession::SetOnReceiveMsgCallback(WebsocketSession::OnReceiveMsgCallback callback) {
  receive_msg_callback_ = std::move(callback);
}

void WebsocketSession::SetOnAbnormalDisconnectCallback(WebsocketSession::OnAbnormalDisconnectCallback callback) {
  connected_ = false;
  abnormal_disconnect_callback_ = std::move(callback);
}


void WebsocketSession::OnSend(const std::shared_ptr<std::string const>& msg) {
  queue_.push_back(msg);

  if (queue_.size() > 1)
    return;

  ws_.async_write(
      boost::asio::buffer(*queue_.front()),
      boost::beast::bind_front_handler(
          &WebsocketSession::OnWrite,
          shared_from_this()));
}

void WebsocketSession::OnWrite(const boost::beast::error_code& ec, std::size_t) {
  if (ec) {
    LOG(ERROR) << ">>>>>>>>>> WebsocketSession write error:" << ec.message();
    return;
  }

  queue_.erase(queue_.begin());

  if (!queue_.empty()) {
    ws_.async_write(
        boost::asio::buffer(*queue_.front()),
        boost::beast::bind_front_handler(
            &WebsocketSession::OnWrite,
            shared_from_this()));
  }
}

void WebsocketSession::DoClose() {
  if (ws_.is_open()) {
    ws_.close("close websocket");
  }
}

void WebsocketSession::ProcessDisconnect(const std::string& reason) {
  if (!do_disconnect_ && abnormal_disconnect_callback_) {
    LOG(ERROR) << ">>>>>>>>>> WebsocketSession error:" << reason;
    abnormal_disconnect_callback_(reason);
  }
}

bool WebsocketSession::IsConnected() {
  return connected_;
}

}