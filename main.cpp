#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>

#include <glog/logging.h>

#include "thread/thread_pool.h"
#include "core/licode_signaling.h"
#include "core/licode_token_creator.h"
#include "messenger/websocket_session.h"

using namespace nbaiot;

int main() {
  LOG(INFO) << "Hello licode !!!" << std::endl;

  auto pool = std::make_shared<ThreadPool>(1);
  pool->Start();

  auto token = LicodeTokenCreator::SyncCreate("http://106.53.67.18:3001/createToken/",
                                              "jack",
                                              3000);

  if (!token.Isvalid()) {
    LOG(ERROR) << ">>>>>> create token failed";
    return -1;
  }

  auto signaling = std::make_shared<LicodeSignaling>(pool->GetLessUsedWorker());
  signaling->SetOnSignalingReadyCallback([](bool success, const std::string& reason) {
    LOG(INFO) << ">>>>>>>>>> Licode Signaling init success!!!";
  });
  signaling->Init(token);

  std::mutex mutex;
  std::unique_lock<std::mutex> lk(mutex);
  std::condition_variable cv;
  cv.wait(lk);
  return 0;
}