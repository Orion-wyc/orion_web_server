/*
 * @Author       : Orion
 * @Date         : 2022-06-28
 * @copyleft Apache 2.0
 */

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include <condition_variable>
#include <mutex>

namespace webserver {

/* 通过互斥量mutex和条件变量condition_variable实现信号量Semaphore. */
class Semaphore {
 public:
  Semaphore(unsigned long cnt = 0) : count_(cnt){};
  ~Semaphore(){};

  /* 禁止拷贝与赋值, 此处使用delete而非Uncopyable */
  Semaphore(const Semaphore&) = delete;
  Semaphore(Semaphore&&) = delete;
  Semaphore& operator=(const Semaphore&) = delete;
  Semaphore& operator=(Semaphore&&) = delete;

  void Release();
  void Acquire();
  bool TryAcquire();

 private:
  std::mutex mutex_;
  std::condition_variable cv_;
  unsigned long count_ = 0;
};

}  // namespace webserver

#endif