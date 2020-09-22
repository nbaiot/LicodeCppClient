//
// Created by nbaiot@126.com on 2020/8/28.
//

#ifndef RTCGATEWAY_SCHEDULER_H
#define RTCGATEWAY_SCHEDULER_H

#include <map>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <memory>
#include <functional>
#include <condition_variable>

namespace nbaiot {

class Scheduler {

public:
  using Function = std::function<void()>;

  explicit Scheduler(int threadCount);

  ~Scheduler();

  void Stop(bool drain = false);

  void Schedule(Function func, std::chrono::steady_clock::time_point point);

  void ScheduleFromNow(Function func, std::chrono::milliseconds deltaMS);

private:
  void ProcessFunctionQueue();

private:
  std::atomic_int thread_count_;
  std::vector<std::unique_ptr<std::thread>> threads_;
  mutable std::mutex queue_mutex_;
  std::multimap<std::chrono::steady_clock::time_point, Function> function_queue_;
  std::condition_variable queue_schedule_cv_;
  bool stop_requested_;
  bool stop_when_empty_;

};

}

#endif //RTCGATEWAY_SCHEDULER_H
