//
// Created by nbaiot@126.com on 2019/12/24.
//
#ifndef LICODECPPCLIENT_HTTP_DEFINES_H
#define LICODECPPCLIENT_HTTP_DEFINES_H

#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/dynamic_body.hpp>

namespace nbaiot {

using EmptyHttpBody = boost::beast::http::empty_body;
using StringHttpBody = boost::beast::http::string_body;
using FileHttpBody = boost::beast::http::file_body;
using BufferHttpBody = boost::beast::http::buffer_body;
using DynamicHttpBody = boost::beast::http::dynamic_body;

enum HttpProtocol {
  kHTTP_1_0,
  kHTTP_1_1,
  kHTTP_2_0,
};

}

#endif //LICODECPPCLIENT_HTTP_DEFINES_H
