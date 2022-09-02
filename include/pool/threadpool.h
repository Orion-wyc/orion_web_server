/*
 * @Author       : Orion
 * @Date         : 2022-07-11
 * @copyleft Apache 2.0
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>
#include <mutex>
#include <queue>
#include <thread>

namespace webserver {

class ThreadPool {
 public:
  /* 之前没有定义成模板类导致链接时提示multiple definition of ...
     解决方法：
     （1) 定义成模板类，然后模板声明和定义写在一个头文件内；
      (2) 声明+定义的写法，让所有的成员函数都在类内部，实现inline内联；
      (3) Task类别显示指出，即本例的解决方案，头文件写声明和源文件写定义。
  */
  using Task = std::function<void()>;

  explicit ThreadPool(size_t n_threads);

  ~ThreadPool();

  /* 暂时设计为需要手动std::bind()仿函数,后期可改成如下：
   * template <typename F, typename... Args>
   */
  void AddTask(Task &&task);

  /* 拷贝构造函数，并且取消默认父类构造函数 */
  ThreadPool(const ThreadPool &) = delete;

  /* 拷贝构造函数，允许右值引用 */
  ThreadPool(const ThreadPool &&) = delete;

  /* 赋值操作 */
  ThreadPool &operator=(const ThreadPool &) = delete;

  /* 赋值操作 */
  ThreadPool &operator=(const ThreadPool &&) = delete;

 private:
  /* Pool的成员closed和tasks需要加锁访问 */
  struct Pool {
    bool closed;
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<Task> tasks;
    Pool(bool status = false) : closed(false) {}
  };

  std::shared_ptr<Pool> pool_;
  std::list<std::thread> workers_;
};

}  // namespace webserver

#endif