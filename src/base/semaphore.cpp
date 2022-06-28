/*
 * @Author       : Orion
 * @Date         : 2022-06-28
 * @copyleft Apache 2.0
 */

#include "base/semaphore.h"

namespace webserver {

void Semaphore::Acquire() {
  std::unique_lock<decltype(mutex_)> lock(mutex_);
  while (count_ == 0) {
    cv_.wait(lock);
  }
  --count_;
}

bool Semaphore::TryAcquire() {
  std::lock_guard<decltype(mutex_)> lock(mutex_);
  if (count_ > 0) {
    --count_;
    return true;
  }
  return false;
}

void Semaphore::Release() {
  std::lock_guard<decltype(mutex_)> lock(mutex_);
  ++count_;
  cv_.notify_one();
}

}  // namespace webserver
