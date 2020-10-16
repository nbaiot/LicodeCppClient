#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>

#include <glog/logging.h>


#include "thread/thread_pool.h"
#include "core/licode_signaling.h"
#include "core/licode_nuve_api.h"
#include "core/licode_room.h"

#include "core/webrtc_wrapper.h"

using namespace nbaiot;

int main() {
  LOG(INFO) << "Hello licode !!!" << std::endl;

  WebrtcWrapper::Instance()->Init();
  WebrtcWrapper::SetWebrtcLogLevel(WebrtcWrapper::kWarning);

  auto pool = std::make_shared<ThreadPool>(1);
  pool->Start();

  auto nuve = std::make_shared<LicodeNuveApi>("5f890e820d2a8b6d6234e38f",
                                              "11944", "10.18.49.172",
                                              3000);
  auto rooms = nuve->SyncListRoom();

  if (!rooms.has_value())
    return -1;

  auto listRooms = rooms.value();

  std::string exampleRoomId;
  for (const auto& room: listRooms) {
    if (room->Name() == "basicExampleRoom")
      exampleRoomId = room->Id();
  }

  if (exampleRoomId.empty())
    return -1;


  /// role: presenter, viewer, viewerWithData
  auto token = nuve->SyncCreateToken(exampleRoomId, "shajia", "presenter");

  if (!token.has_value())
    return -1;

  auto licodeToken = token.value();

  auto room = std::make_shared<LicodeRoom>(pool->GetLessUsedWorker(), licodeToken);

  room->Join();

  std::mutex mutex;
  std::unique_lock<std::mutex> lk(mutex);
  std::condition_variable cv;
  cv.wait(lk);
  return 0;
}