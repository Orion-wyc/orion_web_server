/*
 * @Author       : Orion
 * @Date         : 2022-09-09
 * @copyleft Apache 2.0
 */

#ifndef TASK_GRAPH_H
#define TASK_GRAPH_H

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace webserver {

/* TaskInfo 存储单个任务的基本信息 */
struct TaskInfo {
  std::string task_name;  // 任务名
  std::string shell_cmd;  // 任务命令
  int max_attempts;       // 最多失败尝试次数
  TaskInfo(const std::string &task_name, const std::string &shell_cmd,
           int max_attempts)
      : task_name(task_name),
        shell_cmd(shell_cmd),
        max_attempts(max_attempts) {}
};

class TaskGraph {
 public:
  // 析构函数中销毁 TaskNode
  ~TaskGraph();
  void DestroyGraph();

  // 向 Graph 中添加一个 TaskNode
  bool AddTask(const std::string &task_name,
               const std::vector<std::string> &deps,
               const std::string &shell_cmd, int max_attempts);

  // 初始化 Graph：检查任务是否存在、判环、初始化任务列表
  bool InitGraph();

  // 获取无依赖的待执行任务
  void GetTodoTasks(std::vector<TaskInfo *> &todos);

  // 获取任务信息
  TaskInfo *GetTaskInfo(const std::string &task_name);

  // 将任务 task_name 标记为完成状态
  bool MarkTaskDone(const std::string &task_name);

  // 输出当前TaskGraph中的任务执行情况
  void PrintCurrentGraph(std::ostream &ostream);

 private:
  struct TaskNode {
    TaskInfo info;                          // 调度系统需要的任务信息
    std::map<std::string, bool> out_edges;  // 本任务依赖的其他任务、完成状况
    std::map<std::string, bool> in_edges;  // 依赖本任务的其他任务、完成状况
    int out_degree;  // 剩余依赖任务（前驱任务）
    int in_degree;   // 剩余被依赖任务（后继任务）
    bool done;       // 任务是否完成
    TaskNode(const std::string &task_name, const std::vector<std::string> &deps,
             const std::string &shell_cmd, int max_attempts)
        : info(task_name, shell_cmd, max_attempts),
          out_degree(0),
          in_degree(0),
          done(false) {
      // std::vector<std::string>::const_iterator = deps.begin()
      for (auto iter = deps.begin(); iter != deps.end(); ++iter) {
        // std::pair<std::map<std::string, bool>::iterator, bool>
        auto ret = out_edges.insert(make_pair(*iter, false));
        if (ret.second) {
          ++out_degree;
        }
      }
    }
  };

  // 在图数据结构中维护所有任务
  typedef std::map<std::string, TaskNode *> Graph;
  Graph graph_;

  // 依赖完全, 可以立即执行的任务
  std::set<std::string> todo_tasks_;
};

}  // namespace webserver

#endif  // TASK_GRAPH_H