//
// Created by nbaiot@126.com on 2020/8/28.
//

#ifndef RTCGATEWAY_THREAD_POOL_H
#define RTCGATEWAY_THREAD_POOL_H

#include <memory>
#include <vector>

namespace nbaiot {

class Worker;
class Scheduler;

class ThreadPool {

public:
  explicit ThreadPool(int count);

  ThreadPool(std::shared_ptr<Scheduler> scheduler, int count);

  ~ThreadPool();

  void Start();

  void Stop();

  std::shared_ptr<Worker> GetLessUsedWorker();

private:
  std::vector<std::shared_ptr<Worker>> workers_;
  std::shared_ptr<Scheduler> scheduler_;
};

}

#endif //RTCGATEWAY_THREAD_POOL_H
