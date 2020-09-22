#include <iostream>
#include <thread>
#include <glog/logging.h>
#include "messenger/http_session.h"

/// https://www.boost.org/doc/libs/1_74_0/libs/beast/doc/html/beast/examples.html#beast.examples.servers_advanced
int main() {
  std::cout << "Hello, World!" << std::endl;
  boost::asio::io_context ioc;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard{ioc.get_executor()};
  auto thread = std::make_shared<std::thread>([&ioc]() {
    ioc.run();
  });
  auto http = std::make_shared<nbaiot::HttpSession>(ioc);
  http->Connect("106.53.67.18", 3001, 3000, [](bool success){
    LOG(INFO) << "connnect:" << success;
  });

  ioc.run();
  return 0;
}