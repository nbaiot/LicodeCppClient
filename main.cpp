#include <iostream>
#include <thread>
#include <glog/logging.h>
#include "core/licode_token_creator.h"
#include <nlohmann/json.hpp>
#include <boost/asio/io_context.hpp>

#include "messenger/websocket_session.h"

/// https://www.boost.org/doc/libs/1_74_0/libs/beast/doc/html/beast/examples.html#beast.examples.servers_advanced
using namespace nbaiot;
int main() {
  std::cout << "Hello, World!" << std::endl;
  boost::asio::io_context ioc;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard{ioc.get_executor()};

//  auto tokenCreator = std::make_shared<LicodeTokenCreator>();
//  tokenCreator->SyncCreate("http://106.53.67.18:3002/createToken/", 3000);
  auto websocketSession = std::make_shared<WebsocketSession>("ws://106.53.67.18:8080/socket.io/?EIO=3&transport=websocket");
  websocketSession->Connect();
  ioc.run();
  return 0;
}