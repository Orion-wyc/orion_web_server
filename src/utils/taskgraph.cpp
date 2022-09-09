/*
 * @Author       : Orion
 * @Date         : 2022-09-09
 * @copyleft Apache 2.0
 */

#include "utils/taskgraph.h"

#include <iostream>
#include <stack>

namespace webserver {

TaskGraph::~TaskGraph() {
  // Graph::iterator = graph_.begin()
  for (auto& item : graph_) {
    delete item.second;
    item.second = nullptr;
  }
}

bool TaskGraph::AddTask(const std::string& task_name,
                        const std::vector<std::string>& deps,
                        const std::string& shell_cmd, int max_attempts) {
  // Graph::iterator
  auto iter = graph_.find(task_name);
  if (iter != graph_.end()) {
    return false;
  }

  TaskNode* node = new TaskNode(task_name, deps, shell_cmd, max_attempts);
  graph_.insert(std::make_pair(task_name, node));

  return true;
}

// 初始化拓扑
bool TaskGraph::InitGraph() {
  // 逆拓扑栈
  std::stack<std::string> top_stack;
  // 每个任务的出度
  std::map<std::string, int> tmp_out_degrees;

  // 检查是否所有依赖任务都存在
  for (const auto& item : graph_) {
    TaskNode* src_node = item.second;
    for (const auto& dst_node : src_node->out_edges) {
      // 在任务列表中查找依赖任务是否存在
      auto dst_iter = graph_.find(dst_node.first);
      // 依赖任务不存在
      if (dst_iter == graph_.end()) {
        return false;
      }
      // 获取当前依赖任务的节点并添加入边（即添dst的后继任务节点）
      TaskNode* pdst = dst_iter->second;
      pdst->in_edges.insert(std::make_pair(item.first, false));
      ++pdst->in_degree;
    }
    tmp_out_degrees[item.first] = src_node->out_degree;
    // 没有依赖任务的进入逆拓扑栈（出度为0直接入栈）
    if (src_node->out_degree == 0) {
      top_stack.push(item.first);
    }
  }

  // 检查图中是否有环
  size_t top_cnt = 0;
  while (!top_stack.empty()) {
    std::string task_name = top_stack.top();
    top_stack.pop();
    ++top_cnt;

    TaskNode* dst_node = graph_[task_name];
    // 为每个入边的源点的出度-1
    for (const auto& src_node : dst_node->in_edges) {
      if (--tmp_out_degrees[src_node.first] == 0) {
        // 出度减少为0, 加入栈中
        top_stack.push(src_node.first);
      }
    }
  }
  if (top_cnt != graph_.size()) {  // 存在环
    return false;
  }

  // 生成初始待办任务
  for (const auto& item : graph_) {
    TaskNode* node = item.second;
    if (node->out_degree == 0) {
      todo_tasks_.insert(item.first);
    }
  }
  return true;
}

void TaskGraph::GetTodoTasks(std::vector<TaskInfo*>& todo) {
  for (const std::string& name : todo_tasks_) {
    TaskNode* node = graph_[name];
    todo.push_back(&node->info);
  }
}

TaskInfo* TaskGraph::GetTaskInfo(const std::string& task_name) {
  return &graph_[task_name]->info;
}

bool TaskGraph::MarkTaskDone(const std::string& task_name) {
  auto iter = todo_tasks_.find(task_name);
  if (iter == todo_tasks_.end()) {
    return false;
  }

  // 删除task_name任务
  todo_tasks_.erase(iter);

  // 标记为已完成
  TaskNode* node = graph_[task_name];
  node->done = true;

  // 找到入边的源点, 减少它们的出度
  // 更新task_name在后继任务依赖数
  for (const auto& dst_node : node->in_edges) {
    TaskNode* src_node = graph_[dst_node.first];
    // 标记出边删除
    src_node->out_edges[task_name] = true;
    if (--src_node->out_degree == 0) {
      // 出度为0, 依赖完全, 进入待办
      todo_tasks_.insert(dst_node.first);
    }
  }

  // 找到出边的终点, 减少它们的入度
  // 此处确保task_name在前驱任务还未完成的情况下被正确删除
  for (const auto& src_node : node->out_edges) {
    TaskNode* dst_node = graph_[src_node.first];
    // 标记入边删除
    dst_node->in_edges[task_name] = true;
    --dst_node->in_degree;
  }

  return true;
}

void TaskGraph::PrintCurrentGraph(std::ostream& ostream) {
  ostream << "---------------" << std::endl;
  for (const auto& item : graph_) {
    TaskNode* node = item.second;

    ostream << "TASK NAME: " << item.first << std::endl;
    ostream << "> SHELL CMD: " << node->info.shell_cmd << std::endl;
    ostream << "> MAX ATTEMPTS" << node->info.max_attempts << std::endl;
    ostream << "> DONE: " << (node->done ? "YES" : "NO") << std::endl;
    ostream << "> ANTECEDENT TASK: ";
    for (const auto& dst_node : node->out_edges) {
      if (!dst_node.second) {
        ostream << dst_node.first << " ";
      }
    }
    ostream << std::endl;

    ostream << "> SUCCEED TASKS: ";
    for (const auto& src_node : node->in_edges) {
      if (!src_node.second) {
        ostream << src_node.first << " ";
      }
    }
    ostream << "\n\n";
  }
  ostream << "---------------" << std::endl;
}

}  // namespace webserver
