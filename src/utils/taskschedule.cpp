/*
 * @Author       : Orion
 * @Date         : 2022-09-09
 * @copyleft Apache 2.0
 */

#include "utils/taskschedule.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

#include <iostream>

namespace webserver {

// 初始化调度单元
ScheduleUnit::ScheduleUnit(TaskGraph* graph)
    : graph_(graph), final_fail_(false) {
  std::vector<TaskInfo*> todo_tasks;

  graph_->GetTodoTasks(todo_tasks);
  for (size_t i = 0; i < todo_tasks.size(); ++i) {
    all_.insert(todo_tasks[i]->task_name);
    todo_.insert(todo_tasks[i]->task_name);
  }
}

// 销毁调度单元
ScheduleUnit::~ScheduleUnit() { delete graph_; }

// 任务执行成功
void ScheduleUnit::Success(const std::string& task_name) {
  doing_.erase(task_name);
  done_.insert(task_name);

  graph_->MarkTaskDone(task_name);

  std::vector<TaskInfo*> todo_tasks;
  graph_->GetTodoTasks(todo_tasks);
  for (size_t i = 0; i < todo_tasks.size(); ++i) {
    if (!all_.count(todo_tasks[i]->task_name)) {
      all_.insert(todo_tasks[i]->task_name);
      todo_.insert(todo_tasks[i]->task_name);
    }
  }
}

// 任务执行失败
void ScheduleUnit::Fail(const std::string& task_name) {
  doing_.erase(task_name);
  todo_.insert(task_name);

  int fail_cnt = 0;
  ;

  if (!fail_times_.count(task_name)) {
    fail_cnt = fail_times_[task_name] = 0;
  } else {
    fail_cnt = fail_times_[task_name]++;
  }

  TaskInfo* info = graph_->GetTaskInfo(task_name);
  if (fail_cnt >= info->max_attempts) {  // 超过任务指定失败次数
    final_fail_ = true;
  }
}

// 提供资源, 返回要执行的任务
void ScheduleUnit::Offer(int offer_cnt,
                         std::vector<TaskInfo*>& to_schedule_tasks) {
  if (final_fail_) {  // 单元失败, 不再继续调度更多任务
    return;
  }
  for (int i = 0; i < offer_cnt; ++i) {
    if (todo_.begin() == todo_.end()) {
      break;
    }
    std::string task_name = *todo_.begin();
    to_schedule_tasks.push_back(graph_->GetTaskInfo(task_name));
    todo_.erase(task_name);
    doing_.insert(task_name);
  }
}

// 调度单元是否完成
bool ScheduleUnit::Done() {
  // 全部调度成功
  if (todo_.empty() && doing_.empty()) {
    return true;
  }
  // 调度失败超过次数, 没有其他未完成的任务, 立即结束调度
  if (doing_.empty() && final_fail_) {
    return true;
  }
  return false;
}

// 初始化调度器
TaskSchedule::TaskSchedule(int max_parallel)
    : max_parallel_(max_parallel), left_offer_(max_parallel) {}

// 添加新的调度任务
void TaskSchedule::AddGraph(TaskGraph* graph) {
  all_units_.insert(new ScheduleUnit(graph));
  graph->PrintCurrentGraph(std::cout);
}

// 检查执行单元状态
void TaskSchedule::CheckUnitStatus(ScheduleUnit* unit) {
  // DEBUG
  unit->GetGraph()->PrintCurrentGraph(std::cout);

  if (unit->Done()) {
    all_units_.erase(unit);
    delete unit;
  }
}

// 启动调度
//
// 为了简化采用了轮询形式,
// 建议生产环境采用epoll+pipe监听增加拓扑和任务执行完成的事件,
//
void TaskSchedule::Run() {
  do {
    // 1, 检测结束的任务与单元
    do {
      int status = 0;
      pid_t pid = waitpid(-1, &status, WNOHANG);
      if (pid > 0) {
        if (WIFEXITED(status)) {
          bool success = !WEXITSTATUS(status);
          std::pair<std::string, ScheduleUnit*>& info = schedule_infos_[pid];
          if (success) {
            info.second->Success(info.first);
          } else {
            info.second->Fail(info.first);
          }
          ++left_offer_;
          ScheduleUnit* unit = info.second;
          schedule_infos_.erase(pid);
          CheckUnitStatus(unit);
        }
      } else if (pid == 0 ||
                 errno == ECHILD) {  // 当前没有其他子进程退出, 或者没有子进程了
        break;
      }
    } while (true);

    // 2, 配额offer给调度单元
    std::vector<TaskInfo*> to_schedule;
    for (std::set<ScheduleUnit*>::iterator iter = all_units_.begin();
         iter != all_units_.end(); ++iter) {
      if (!left_offer_) {  // 没有配额
        break;
      }

      ScheduleUnit* unit = *iter;
      to_schedule.clear();
      unit->Offer(left_offer_, to_schedule);
      if (to_schedule.size()) {
        left_offer_ -= to_schedule.size();
      }

      // 尝试启动每个任务, 多进程并行处理
      for (size_t i = 0; i < to_schedule.size(); ++i) {
        pid_t pid = fork();
        if (pid == -1) {
          ++left_offer_;
          unit->Fail(to_schedule[i]->task_name);
          CheckUnitStatus(unit);
        } else if (pid > 0) {  // 父进程
          schedule_infos_.insert(
              make_pair(pid, make_pair(to_schedule[i]->task_name, unit)));
        } else {  // 子进程
          int nullFd = open("/dev/null", O_APPEND);
          if (nullFd != -1) {
            dup2(nullFd, STDOUT_FILENO);
            dup2(nullFd, STDERR_FILENO);
            close(nullFd);
          }
          if (execlp("/bin/bash", "bash", "-c",
                     to_schedule[i]->shell_cmd.c_str(),
                     NULL) == -1) {  // 调起失败,exit(-1)
            exit(-1);
          }
        }
      }
    }

    // 3, 所有单元结束，退出
    if (all_units_.empty()) {
      break;
    }
  } while (true);
}

}  // namespace webserver