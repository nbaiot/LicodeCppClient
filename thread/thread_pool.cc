//
// Created by nbaiot@126.com on 2020/8/28.
//

#include "thread_pool.h"
#include "worker.h"
#include "scheduler.h"

namespace nbaiot {

ThreadPool::ThreadPool(int count) {
  scheduler_ = std::make_shared<Scheduler>(2);
  workers_.reserve(count);
  for (int i = 0; i < count; ++i) {
    workers_.push_back(std::make_shared<Worker>(scheduler_));
  }
}

ThreadPool::ThreadPool(std::shared_ptr<Scheduler> scheduler, int count) : scheduler_(std::move(scheduler)) {
  workers_.reserve(count);
  for (int i = 0; i < count; ++i) {
    workers_.push_back(std::make_shared<Worker>(scheduler_));
  }
}

ThreadPool::~ThreadPool() {
  Stop();
}

void ThreadPool::Start() {
  for(auto & it : workers_) {
    it->Start();
  }
}

void ThreadPool::Stop() {
  for(auto & it : workers_) {
    it->Stop();
  }
}

std::shared_ptr<Worker> ThreadPool::GetLessUsedWorker() {
  auto better = workers_.front();
  for (const auto& worker : workers_) {
    if (better.use_count() > worker.use_count()) {
      better = worker;
    }
  }
  return better;
}

}