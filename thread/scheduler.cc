//
// Created by nbaiot@126.com on 2020/8/28.
//

#include "scheduler.h"

#include <glog/logging.h>

#include <utility>

namespace nbaiot {

Scheduler::Scheduler(int threadCount)
    : thread_count_(threadCount), stop_requested_(false), stop_when_empty_(false) {
  for (int i = 0; i < threadCount; ++i) {
    threads_.push_back(std::make_unique<std::thread>([this]() {
        ProcessFunctionQueue();
    }));
  }
}

Scheduler::~Scheduler() {
  Stop();
  LOG_ASSERT(thread_count_ == 0);
}

void Scheduler::Stop(bool drain) {
  {
    std::unique_lock<std::mutex> lk(queue_mutex_);
    if (drain) {
      stop_when_empty_ = true;
    } else {
      stop_requested_ = true;
    }
  }
  queue_schedule_cv_.notify_all();

  for (const auto& it: threads_) {
    if (it && it->joinable()) {
      it->join();
    }
  }
}

void Scheduler::Schedule(Function func, std::chrono::steady_clock::time_point point) {
  {
    std::unique_lock<std::mutex> lk(queue_mutex_);
    function_queue_.emplace(point, std::move(func));
  }
  queue_schedule_cv_.notify_one();
}

void Scheduler::ScheduleFromNow(Function func, std::chrono::milliseconds deltaMS) {
  Schedule(std::move(func), std::chrono::steady_clock::now() + deltaMS);
}

void Scheduler::ProcessFunctionQueue() {

  std::unique_lock<std::mutex> lk(queue_mutex_);

  do {
    /// blocking
    while (!stop_requested_ && !stop_when_empty_ && function_queue_.empty()) {
      queue_schedule_cv_.wait(lk);
    }

    /// when notify by Schedule()
    auto time = function_queue_.empty() ? std::chrono::steady_clock::now() : function_queue_.begin()->first;

    /// wait util first function to exec
    while (!stop_requested_ && !function_queue_.empty() &&
           queue_schedule_cv_.wait_until(lk, time) != std::cv_status::timeout) {
      time = function_queue_.empty() ? std::chrono::steady_clock::now() : function_queue_.begin()->first;
    }

    if (stop_requested_) {
      break;
    }

    if (function_queue_.empty()) {
      continue;
    }

    auto f = function_queue_.begin()->second;
    function_queue_.erase(function_queue_.begin());

    lk.unlock();
    try {
      f();
    } catch (...) {
      --thread_count_;
      LOG(FATAL) << "An exception has been thrown inside Scheduler";
    }
    lk.lock();

  } while (!stop_requested_ && !(stop_when_empty_ && function_queue_.empty()));

  --thread_count_;
}

}