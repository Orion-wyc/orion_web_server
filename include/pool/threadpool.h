/*
 * @Author       : Orion
 * @Date         : 2022-07-11
 * @copyleft Apache 2.0
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>
#include <mutex>
#include <queue>
#include <thread>

namespace webserver {

class ThreadPool {
 public:
  explicit ThreadPool(size_t n_threads);

  ~ThreadPool();

  template <typename F>
  void AddTask(F &&task);

 private:
  /* Pool的成员closed和tasks需要加锁访问 */
  struct Pool {
    bool closed;
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<std::function<void()>> tasks;
    Pool(bool status = false) : closed(false) {}
  };

  std::shared_ptr<Pool> pool_;
  std::list<std::thread> workers_;
};

ThreadPool::ThreadPool(size_t n_threads)
    : pool_(std::make_shared<Pool>(false)) {
  for (size_t i = 0; i < n_threads; ++i) {
    /* warning: lambda capture initializers only available
     * with -std=c++14 or -std=gnu++14
     * workers_.emplace_back([pool = pool_]() {
     */
    workers_.emplace_back([this]() {
      std::shared_ptr<Pool> pool = this->pool_;
      while (true) {
        std::function<void()> task;
        {
          std::unique_lock<decltype(pool->mtx)> lock(pool->mtx);
          pool->cv.wait(
              lock, [pool]() { return pool->closed || !pool->tasks.empty(); });

          if (pool->closed && pool->tasks.empty()) {
            return;
          }

          task = std::move(pool->tasks.front());
          pool->tasks.pop();
        }
        task();
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  if (pool_) {
    std::lock_guard<decltype(pool_->mtx)> lock(pool_->mtx);
    pool_->closed = true;
  }
  pool_->cv.notify_all();

  for (std::thread &worker : workers_) {
    worker.join();
  }
}

template <typename F>
void ThreadPool::AddTask(F &&task) {
  if (pool_->closed) {
    throw std::runtime_error("add task into a closed thread pool");
  }
  {
    std::lock_guard<decltype(pool_->mtx)> lock(pool_->mtx);
    pool_->tasks.emplace(std::forward<F>(task));
  }
  pool_->cv.notify_one();
}

}  // namespace webserver

#endif