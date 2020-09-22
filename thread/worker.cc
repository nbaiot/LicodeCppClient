//
// Created by nbaiot@126.com on 2020/8/28.
//

#include "worker.h"

#include <future>
#include <glog/logging.h>
#include <boost/asio/dispatch.hpp>

#include "scheduler.h"

namespace nbaiot {

ScheduledTaskReference::ScheduledTaskReference() : cancelled_(false) {

}

bool ScheduledTaskReference::IsCancelled() {
  return cancelled_;
}

void ScheduledTaskReference::Cancel() {
  cancelled_ = true;
}

Worker::Worker(std::weak_ptr<Scheduler> scheduler) : scheduler_(std::move(scheduler)), running_(false) {

}

void Worker::Start() {
  if (running_)
    return;

  running_ = true;

  ioc_.reset();
  guard_ =
      std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(ioc_.get_executor());

  auto promise = std::make_shared<std::promise<void>>();
  work_thread_ = std::make_unique<std::thread>([this, promise]() {
      work_thread_id_ = std::this_thread::get_id();
      promise->set_value();
      ioc_.run();
      /// TODO: fixme
      // work_thread_id_ = std::thread::id();
  });

  promise->get_future().wait();
}

void Worker::Stop() {
  if (!running_)
    return;
  running_ = false;

  if (guard_) {
    guard_->reset();
  }

  if (work_thread_ && work_thread_->joinable()) {
    work_thread_->join();
  }

  ioc_.stop();

}

bool Worker::Running() {
  return running_;
}

std::thread::id Worker::ThreadId() {
  return work_thread_id_;
}

void Worker::PostTask(Task task) {
  boost::asio::dispatch(ioc_, task);
}

std::shared_ptr<ScheduledTaskReference>
Worker::ScheduleFromNow(Task task, std::chrono::steady_clock::duration duration) {
  auto ref = std::make_shared<ScheduledTaskReference>();
  auto scheduler = scheduler_.lock();
  if (!scheduler) {
    return ref;
  }

  scheduler->ScheduleFromNow(SafeTask([ref, task](const std::shared_ptr<Worker>& worker) {
      worker->PostTask(worker->SafeTask([ref, task](const std::shared_ptr<Worker>& worker) {
          if (ref->IsCancelled())
            return;
          try {
            task();
          } catch (std::exception& e) {
            LOG(ERROR) << "worker exec task error:" << e.what();
          }
      }));
  }), std::chrono::duration_cast<std::chrono::milliseconds>(duration));
  return std::shared_ptr<ScheduledTaskReference>();
}

void Worker::Unschedule(const std::shared_ptr<ScheduledTaskReference>& ref) {
  ref->Cancel();
}

void Worker::ScheduleEvery(PeriodTask task, std::chrono::steady_clock::duration duration) {
  ScheduleEvery(std::move(task), duration, duration);
}

std::function<void()> Worker::SafeTask(const std::function<void(std::shared_ptr<Worker>)>& f) {
  auto weakSelf = weak_from_this();
  return [f, weakSelf]() {
      if (auto self = weakSelf.lock()) {
        f(self);
      }
  };
}

void Worker::ScheduleEvery(PeriodTask task, std::chrono::steady_clock::duration period,
                           std::chrono::steady_clock::duration nextDelay) {

  auto start = std::chrono::steady_clock::now();
  ScheduleFromNow(SafeTask([task, start, period, nextDelay](const std::shared_ptr<Worker>& worker) {
      try {
        if(task()) {
          auto delay = period - (std::chrono::steady_clock::now() - start - nextDelay);
          worker->ScheduleEvery(task, period, delay);
        }
      } catch (std::exception& e) {
        LOG(ERROR) << "worker exec task error:" << e.what();
      }

  }), std::max(nextDelay, std::chrono::steady_clock::duration(0)));
}


}