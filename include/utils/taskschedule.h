/*
 * @Author       : Orion
 * @Date         : 2022-09-09
 * @copyleft Apache 2.0
 */
#ifndef TASK_SCHEDULE_H
#define TASK_SCHEDULE_H

#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "utils/taskgraph.h"

namespace webserver {

class TaskInfo;

// 调度单元, 管理单个拓扑
class ScheduleUnit {
 public:
  // 初始化调度单元
  ScheduleUnit(TaskGraph* graph);
  // 销毁调度单元
  ~ScheduleUnit();
  // 任务执行成功
  void Success(const std::string& task_name);
  // 任务执行失败
  void Fail(const std::string& task_name);
  // 提供资源, 返回要执行的任务
  void Offer(int offer_cnt, std::vector<TaskInfo*>& to_schedule_tasks);
  // 调度单元是否完成(全部成功或者某个任务失败)
  bool Done();
  // 获取graph
  TaskGraph* GetGraph() { return graph_; }

 private:
  // 当todo_和doing_均为空的时候, 整个拓扑调度完成
  TaskGraph* graph_;

  std::set<std::string> all_;    // all_=todo_+doing_+done_
  std::set<std::string> todo_;   // 等待执行的任务
  std::set<std::string> doing_;  // 正在执行的任务
  std::set<std::string> done_;   // 执行完成的任务

  // 是否某个任务彻底失败
  bool final_fail_;
  // 每个任务的失败次数
  std::map<std::string, int> fail_times_;
};

// 调度器, 管理多个拓扑
class TaskSchedule {
 public:
  // 初始化调度器
  TaskSchedule(int max_parallel);

  // 添加新的调度任务
  void AddGraph(TaskGraph* graph);

  // 启动调度
  void Run();

 private:
  // 检查执行单元状态
  void CheckUnitStatus(ScheduleUnit* unit);

  // 限制最大并发任务数
  int max_parallel_;
  // 剩余并发配额
  int left_offer_;

  // 维护所有的调度单元
  std::set<ScheduleUnit*> all_units_;

  // 每个任务在1个子进程中执行, 记录上下文信息
  std::map<pid_t, std::pair<std::string, ScheduleUnit*> > schedule_infos_;
};

}  // namespace webserver

#endif  // TASK_SCHEDULE_H