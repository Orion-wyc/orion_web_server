/*
 * @Author       : Orion
 * @Date         : 2022-07-11
 * @copyleft Apache 2.0
 */

#include "pool/threadpool.h"

namespace webserver {

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

/* 此处也遇到一个问题，即std::bind无法转化为std::function<void()>类型，
   原因时因为调用AddTask(std::bind(xxxx))时，括号内产生的函数对象是个右
   值（即没有地址），而之前的函数定义AddTask(Task &task)并要求传入的参数
   是左值，由此编译报错。解决方法有两个：
   （1）使用完美转化；
   （2）使用AddTask(cosnt Task &task)，会自动构造一个Task函数对象
   参考此处：https://blog.csdn.net/qq_26973089/article/details/85122861

*/
void ThreadPool::AddTask(Task &&task) {
  if (pool_->closed) {
    throw std::runtime_error("add task into a closed thread pool");
  }
  {
    std::lock_guard<decltype(pool_->mtx)> lock(pool_->mtx);
    pool_->tasks.emplace(std::forward<Task>(task));
  }
  pool_->cv.notify_one();
}

}  // namespace webserver