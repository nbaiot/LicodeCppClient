//
// Created by nbaiot@126.com on 2020/9/23.
//
#ifndef LICODECPPCLIENT_HTTP_REQUEST_H
#define LICODECPPCLIENT_HTTP_REQUEST_H

#include <map>
#include <string>

#include <glog/logging.h>
#include <boost/beast/http/message.hpp>

#include "http_url.h"
#include "http_defines.h"

namespace nbaiot {

/**
 * request:
 * Request line
 * Request header
 * Request body
 */
 /// TODO: use std::enable_if
template <class HttpBody>
class HttpRequest {
public:
  HttpRequest() = default;

  HttpRequest<HttpBody>* SetUrl(const HttpUrl& url) {
    url_ = url;
    std::string host = url_.Host() + ":" + std::to_string(url_.Port());
    request_.set(boost::beast::http::field::host, url_.Host());
    request_.target(url_.Url().substr(url_.Scheme().size() + 3 + host.size()));
    return this;
  }

  HttpRequest<HttpBody>* SetUrl(const std::string& url) {
    url_ = HttpUrl(url);
    /// TODO: add check
    std::string host = url_.Host() + ":" + std::to_string(url_.Port());
    request_.set(boost::beast::http::field::host, url_.Host());
    request_.target(url_.Url().substr(url_.Scheme().size() + 3 + host.size()));
    return this;
  }

  HttpRequest<HttpBody>* AddHeader(const std::string& name, const std::string& value) {
    if (name.empty() || value.empty()) {
      LOG(ERROR) << "AddHeader error, name or value empty";
      return this;
    }
    ///LOG(INFO) << boost::beast::http::to_string(boost::beast::http::field::user_agent);
    auto field = boost::beast::http::detail::get_field_table().string_to_field(name);
    if (field == boost::beast::http::field::unknown) {
      LOG(ERROR) << "AddHeader error, invalid header name:" << name;
      return this;
    }
    request_.set(field, value);
    return this;
  }

  HttpRequest<HttpBody>* RemoveHeader(const std::string& name) {
    if (name.empty()) {
      LOG(ERROR) << "RemoveHeader error, name empty";
      return this;
    }
    auto field = boost::beast::http::detail::get_field_table().string_to_field(name);
    if (field == boost::beast::http::field::unknown) {
      LOG(ERROR) << "RemoveHeader error, invalid header name:" << name;
      return this;
    }
    request_.erase(field);
    return this;
  }

  HttpRequest<HttpBody>* SetMethod(const std::string& method) {
    if (method.empty()) {
      LOG(ERROR) << "Method error, method name empty";
      return this;
    }

    auto m = boost::beast::http::string_to_verb(method);
    if (m == boost::beast::http::verb::unknown) {
      LOG(ERROR) << "Method error, invalid method name:" << method;
      return this;
    }
    request_.method(m);
    return this;
  }

  HttpBody& Body() {
    return request_.body();
  }

  std::string Method() {
    return request_.method_string();
  }

  void SetVersion(enum HttpProtocol protocol) {
    switch (protocol) {
      case kHTTP_1_0:
        request_.version(10);
        break;
      case kHTTP_1_1:
        request_.version(11);
        break;
      case kHTTP_2_0:
        request_.version(20);
        break;
    }
  }

  std::string Header(const std::string& name) {
    return request_.at[name];
  }

  HttpUrl Url() {
    return url_;
  }

  void ClearHeaders() {
    request_.clear();
  }

  boost::beast::http::request<HttpBody>& Request() {
    return request_;
  }

private:
  HttpUrl url_{""};
  boost::beast::http::request<HttpBody> request_;
};
}

#endif //LICODECPPCLIENT_HTTP_REQUEST_H
