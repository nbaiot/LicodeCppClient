//
// Created by nbaiot@126.com on 2020/9/23.
//

#ifndef LICODECPPCLIENT_HTTP_RESPONSE_H
#define LICODECPPCLIENT_HTTP_RESPONSE_H

#include <string>
#include <map>

#include <boost/beast/http.hpp>

#include "http_defines.h"

namespace nbaiot {

/// boost::beast::http::response
/// boost::beast::http::header

template<class RequestBody, class ResponseBody>
class HttpResponse {

public:
  HttpResponse() = default;

  void SetRequest(boost::beast::http::request<RequestBody>request) {
    request_ = std::move(request);
  }

  void SetSendRequestAtMillis(int64_t ms) {
    send_request_at_millis_ = ms;
  }

  void SetReceiveResponseAtMillis(int64_t ms) {
    received_response_at_millis_ = ms;
  }

  boost::beast::http::response<ResponseBody>& Response() {
    return response_;
  }

  boost::beast::http::request<RequestBody>& Request() {
    return request_;
  }

  int64_t SendRequestAtMillis() {
    return send_request_at_millis_;
  }

  int64_t ReceiveResponseAtMillis() {
    return received_response_at_millis_;
  }

private:
  int64_t send_request_at_millis_{0};
  int64_t received_response_at_millis_{0};
  boost::beast::http::request<RequestBody> request_;
  boost::beast::http::response<ResponseBody> response_;
};

}

#endif //LICODECPPCLIENT_HTTP_RESPONSE_H
