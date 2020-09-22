//
// Created by nbaiot@126.com on 2020/9/23.
//

#include "websocket_session.h"
#include <boost/asio/strand.hpp>

namespace nbaiot {

WebsocketSession::WebsocketSession(boost::asio::io_context& ioc)
    : resolver_(boost::asio::make_strand(ioc)), ws_(boost::asio::make_strand(ioc)) {

}

WebsocketSession::~WebsocketSession() {

}
}