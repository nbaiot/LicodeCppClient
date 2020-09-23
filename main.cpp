#include <iostream>
#include <thread>
#include <glog/logging.h>
#include "messenger/http_session.h"
#include "messenger/http_url.h"
#include "messenger/http_request.h"

#include <nlohmann/json.hpp>

/// https://www.boost.org/doc/libs/1_74_0/libs/beast/doc/html/beast/examples.html#beast.examples.servers_advanced
using namespace nbaiot;
int main() {
  std::cout << "Hello, World!" << std::endl;
  boost::asio::io_context ioc;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard{ioc.get_executor()};
  auto thread = std::make_shared<std::thread>([&ioc]() {
    ioc.run();
  });
  auto http = std::make_shared<nbaiot::HttpSession>(ioc);
  auto request = std::make_shared<HttpRequest<StringHttpBody>>();
  request->SetUrl("http://106.53.67.18:3001/createToken/");
  request->SetMethod("POST");
  request->SetVersion(kHTTP_1_1);
  request->AddHeader("User-Agent", BOOST_BEAST_VERSION_STRING);
  request->AddHeader("Content-Type", "application/json");
  nlohmann::json data;
  data["mediaConfiguration"] = false;
  data["room"] = false;
  data["type"] = false;
  data["username"] = "jack";
  data["role"] = "presenter";

  request->Body() = "ss";
  http->Post(request);

//  http->Connect("106.53.67.18", 3001, 3000, [](bool success){
//    LOG(INFO) << "connnect:" << success;
//  });

  ioc.run();
  return 0;
}