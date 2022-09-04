/*
 * @Author       : Orion
 * @Date         : 2022-09-03
 * @copyleft Apache 2.0
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <assert.h>

#include <algorithm>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <vector>

#include "utils/logger.h"

namespace webserver {

// 重命名类型，减少代码
using Task = std::function<void()>;
using Clock = std::chrono::high_resolution_clock;
using Msec = std::chrono::milliseconds;
using TimeStamp = Clock::time_point;

/* 二叉堆中的一个节点 */
struct HeapItem {
  int item_id;
  TimeStamp ts_remaining;
  Task cb_func;

  HeapItem(int id, TimeStamp ts, const Task& cb)
      : item_id(id), ts_remaining(ts), cb_func(cb) {}

  bool operator<(const HeapItem& item)  const {
    return ts_remaining < item.ts_remaining;
  }
};

class MinHeapTimer {
 public:
  MinHeapTimer() { heap_.reserve(64); }
  ~MinHeapTimer() { Clear(); };

  // 添加一个元素
  void AddItem(int fd, int timeout, const Task& cb_func);

  // 修改一个节点的时间
  void UpdateItem(int fd, int timeout);

  // 删除指定节点, 并执行回调函数
  void DoWork(int fd);

  // 返回下一个Item时间片耗尽的时间
  int GetNextTick();

  // 清除计时器中所有item
  void Clear();

 private:
  std::vector<HeapItem> heap_;
  // <fd, item_idx>
  std::unordered_map<int, size_t> refs_;

  bool SiftDown_(size_t idx, size_t end);
  bool SiftUp_(size_t idx);
  void SwapItem_(size_t i, size_t j);
  void RemoveItem_(size_t idx);
  void ManageStaleItem_();
};

}  // namespace webserver

#endif