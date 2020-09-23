//
// Created by nbaiot@126.com on 2020/9/23.
//

#ifndef LICODECPPCLIENT_HTTP_SESSION_H
#define LICODECPPCLIENT_HTTP_SESSION_H

#include <any>
#include <memory>
#include <string>
#include <cstdint>
#include <functional>

#include <boost/asio/io_context.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/core/tcp_stream.hpp>

#include <glog/logging.h>

#include "http_request.h"
#include "http_response.h"

namespace nbaiot {

class HttpSession : public std::enable_shared_from_this<HttpSession> {

public:
  using OnConnectCallback = std::function<void(bool success)>;

  explicit HttpSession(boost::asio::io_context& ioc);

  ~HttpSession();

  std::shared_ptr<HttpResponse<StringHttpBody, StringHttpBody>>
  Post(const std::shared_ptr<HttpRequest<StringHttpBody>>& request);

private:
  boost::asio::ip::tcp::resolver resolver_;
  boost::beast::tcp_stream stream_;
};

}


#endif //LICODECPPCLIENT_HTTP_SESSION_H
