/*
 * @Author       : Orion
 * @Date         : 2022-06-30
 * @copyleft Apache 2.0
 *
 * C++中使用模板时，类的声明和定义应当合并到一个文件中；或者在实现文件
 * xxx.cpp的最后提供显式模板实例化声明template class BlockQueue<int>;
 */

#ifndef BLOCKQUEUE_H_
#define BLOCKQUEUE_H_

#include <cassert>
#include <condition_variable>
#include <list>
#include <mutex>

namespace webserver {

template <typename T>
class BlockQueue {
 public:
  explicit BlockQueue(size_t max_capacity = 1024);
  ~BlockQueue();

  void DestroyQueue();

  void Push(const T &item);
  bool Pop(T &item);  // 为保证原子性, 获取队头元素的同时移除元素

  void Flush();  // 将队列中所有未处理的日志写入磁盘

  T &Front();
  T &Back();

  bool Empty();
  bool Full();
  size_t Size();
  size_t Capacity();

 private:
  std::list<T> que_;
  size_t capacity_;
  bool closed_;

  std::mutex mtx_;
  /* 用来通知消费(出队)线程等待(await)还是可以执行(signal) */
  std::condition_variable consumer_cv_;
  /* 用来通知生产(入队)线程等待(await)还是可以执行(signal) */
  std::condition_variable producer_cv_;
};

template <typename T>
BlockQueue<T>::BlockQueue(size_t max_capacity) : capacity_(max_capacity) {
  assert(max_capacity > 0);
  closed_ = false;
}

template <typename T>
BlockQueue<T>::~BlockQueue() {
  DestroyQueue();
}

template <typename T>
void BlockQueue<T>::DestroyQueue() {
  {
    std::lock_guard<decltype(mtx_)> lock(mtx_);
    que_.clear();
    closed_ = true;
  }
  /* 此处要唤起所有阻塞的进程，此时closed_为true，
     队列已清空，调用Pop(T&)返回false */
  producer_cv_.notify_all();
  consumer_cv_.notify_all();
}

template <typename T>
void BlockQueue<T>::Push(const T &item) {
  std::unique_lock<decltype(mtx_)> lock(mtx_);
  while (que_.size() >= capacity_) {
    producer_cv_.wait(lock);
  }
  que_.push_back(item);
  consumer_cv_.notify_one();
}

template <typename T>
bool BlockQueue<T>::Pop(T &item) {
  std::unique_lock<decltype(mtx_)> lock(mtx_);
  while (que_.empty()) {
    consumer_cv_.wait(lock);
    if (closed_) return false;
  }
  item = que_.front();
  que_.pop_front();
  producer_cv_.notify_one();
  return true;
}

template <typename T>
void BlockQueue<T>::Flush() {
  consumer_cv_.notify_one();
}

template <typename T>
T &BlockQueue<T>::Front() {
  std::lock_guard<decltype(mtx_)> lock(mtx_);
  return que_.front();
}

template <typename T>
T &BlockQueue<T>::Back() {
  std::lock_guard<decltype(mtx_)> lock(mtx_);
  return que_.back();
}

template <typename T>
bool BlockQueue<T>::Empty() {
  std::lock_guard<decltype(mtx_)> lock(mtx_);
  return que_.empty();
}

template <typename T>
bool BlockQueue<T>::Full() {
  std::lock_guard<decltype(mtx_)> lock(mtx_);
  return que_.size() >= capacity_;
}

/* 队列是临界区资源，该操作应当是阻塞的 */
template <typename T>
size_t BlockQueue<T>::Size() {
  std::lock_guard<decltype(mtx_)> lock(mtx_);
  return que_.size();
}

/* 该操作应当是非阻塞的 */
template <typename T>
size_t BlockQueue<T>::Capacity() {
  return capacity_;
}

}  // namespace webserver

#endif