#include <iostream>
#include <thread>
#include <glog/logging.h>
#include "core/licode_token_creator.h"
#include <nlohmann/json.hpp>
#include <boost/asio/io_context.hpp>

#include "thread/thread_pool.h"
#include "core/licode_signaling.h"
#include "messenger/websocket_session.h"

using namespace nbaiot;

int main() {
  std::cout << "Hello, World!" << std::endl;
  boost::asio::io_context ioc;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard{ioc.get_executor()};

  auto tokenCreator = std::make_shared<LicodeTokenCreator>();
  auto token = tokenCreator->SyncCreate("http://106.53.67.18:3001/createToken/", 3000);
  auto pool = std::make_shared<ThreadPool>(1);
  pool->Start();

  auto signaling = std::make_shared<LicodeSignaling>(pool->GetLessUsedWorker());
  signaling->Init(token);


  ioc.run();
  return 0;
}