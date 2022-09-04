/*
 * @Author       : Orion
 * @Date         : 2022-09-03
 * @copyleft Apache 2.0
 */

#include "base/timer.h"

namespace webserver {

void MinHeapTimer::AddItem(int fd, int timeout, const Task& cb_func) {
  if (fd < 0) return;
  if (refs_.count(fd) == 0) {
    // 新节点
    size_t idx = heap_.size();
    refs_.insert(std::make_pair(fd, idx));
    heap_.push_back(HeapItem(fd, Clock::now() + Msec(timeout), cb_func));
    SiftUp_(idx);
  } else {
    // 已有节点
    size_t idx = refs_[fd];
    heap_[idx].ts_remaining = Clock::now() + Msec(timeout);
    heap_[idx].cb_func = cb_func;
    // 修改节点后，可能需要向上或向下调整（也可能位置不变）
    if (!SiftDown_(idx, heap_.size())) {
      SiftUp_(idx);
    }
  }
  LOG_INFO("timer fd=%d", fd);
}

void MinHeapTimer::UpdateItem(int fd, int timeout) {
  // 此处写了个bug, 布尔运算的优化
  if (heap_.empty() || refs_.count(fd) == 0) return;
  size_t idx = refs_[fd];
  heap_[idx].ts_remaining = Clock::now() + Msec(timeout);
  SiftDown_(idx, heap_.size());
}

void MinHeapTimer::DoWork(int fd) {
  if (heap_.empty() || refs_.count(fd) == 0) return;

  size_t idx = refs_[fd];
  HeapItem item = heap_[idx];
  item.cb_func();
  RemoveItem_(idx);
}

int MinHeapTimer::GetNextTick() {
  ManageStaleItem_();
  size_t res = -1;
  if (!heap_.empty()) {
    auto timeMsec = std::chrono::duration_cast<Msec>(
        heap_.front().ts_remaining - Clock::now());
    res = timeMsec.count();
    // 理论上清除过期节点后不会是负数, 但是调用过程有额外开开销
    if (res < 0) res = 0;
  }
  LOG_INFO("success timer! (%d, res=%d)", heap_.size(), res);
  return res;
}

void MinHeapTimer::Clear() {
  heap_.clear();
  refs_.clear();
}

/* 私有成员函数 */

bool MinHeapTimer::SiftDown_(size_t idx, size_t end) {
  size_t parent = idx;
  size_t child = parent * 2 + 1;
  while (child < end) {
    if (child + 1 < end && heap_[child + 1] < heap_[child]) ++child;
    if (heap_[parent] < heap_[child]) break;
    SwapItem_(parent, child);
    parent = child;
    child = parent * 2 + 1;
  }
  return parent > idx;  // 确实向下调整了，则无需再向上
}

bool MinHeapTimer::SiftUp_(size_t idx) {
  size_t child = idx;
  size_t parent = (child - 1) / 2;
  while (parent >= 0) {
    if (heap_[parent] < heap_[child]) break;
    SwapItem_(parent, child);
    child = parent;
    parent = (child - 1) / 2;
  }
  return child < idx;
}

void MinHeapTimer::SwapItem_(size_t i, size_t j) {
  // 交换节点并更新索引
  std::swap(heap_[i], heap_[j]);
  refs_[heap_[i].item_id] = i;
  refs_[heap_[j].item_id] = j;
}

void MinHeapTimer::RemoveItem_(size_t idx) {
  size_t i = idx;
  size_t end = heap_.size() - 1;
  if (i < end) {
    SwapItem_(i, end);
    if (!SiftDown_(i, end)) {
      SiftUp_(i);
    }
  }
  refs_.erase(heap_.back().item_id);
  heap_.pop_back();
}

/* 清除超时结点 */
void MinHeapTimer::ManageStaleItem_() {
  while (!heap_.empty()) {
    HeapItem item = heap_.front();
    auto timeMsec =
        std::chrono::duration_cast<Msec>(item.ts_remaining - Clock::now());
    if (timeMsec.count() > 0) break;

    item.cb_func();
    RemoveItem_(0);
    LOG_DEBUG("success remove timer (heap=%d)", heap_.size());
  }
}

}  // namespace webserver