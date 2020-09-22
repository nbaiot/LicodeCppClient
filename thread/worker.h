//
// Created by nbaiot@126.com on 2020/8/28.
//

#ifndef RTCGATEWAY_WORKER_H
#define RTCGATEWAY_WORKER_H

#include <atomic>
#include <memory>
#include <thread>
#include <functional>

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>

namespace nbaiot {

class Scheduler;

class ScheduledTaskReference {
public:
  ScheduledTaskReference();

  bool IsCancelled();

  void Cancel();

private:
  std::atomic_bool cancelled_;
};

class Worker : public std::enable_shared_from_this<Worker> {
public:

  using Task = std::function<void()>;
  using PeriodTask = std::function<bool()>;

  explicit Worker(std::weak_ptr<Scheduler> scheduler);

  void Start();

  void Stop();

  bool Running();

  std::thread::id ThreadId();

  virtual void PostTask(Task task);

  virtual
  std::shared_ptr<ScheduledTaskReference> ScheduleFromNow(Task task, std::chrono::steady_clock::duration duration);

  virtual void Unschedule(const std::shared_ptr<ScheduledTaskReference>& ref);

  virtual void ScheduleEvery(PeriodTask task, std::chrono::steady_clock::duration duration);

private:
  std::function<void()> SafeTask(const std::function<void(std::shared_ptr<Worker>)>& f);

  void ScheduleEvery(PeriodTask task,
                     std::chrono::steady_clock::duration period,
                     std::chrono::steady_clock::duration nextDelay);

private:
  std::weak_ptr<Scheduler> scheduler_;

  boost::asio::io_context ioc_;
  std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> guard_;

  std::unique_ptr<std::thread> work_thread_;
  std::thread::id work_thread_id_;
  std::atomic_bool running_;

};

}

#endif //RTCGATEWAY_WORKER_H
