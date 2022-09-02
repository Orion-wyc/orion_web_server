/*
 * @Author       : Orion
 * @Date         : 2022-06-29
 * @copyleft Apache 2.0
 */

#ifndef SQLCONNPOOL_H_
#define SQLCONNPOOL_H_

#include <mysql/mysql.h>

#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "base/semaphore.h"
// 应当添加log模块

namespace webserver {

/* mysql的多线程连接池, 单例模式
 */
class SqlConnPool {
 public:
  static SqlConnPool *Instance();

  void Initialize(const char *host, int port, const char *user, const char *pwd,
                  const char *db_name, int conn_size);  // 初始化mysql连接池

  MYSQL *AcquireConn();          // 从连接池中获取一个连接
  void ReleaseConn(MYSQL *sql);  // 释放并归还已拥有的连接

  size_t GetFreeCount();  // 连接池中可用线程的数量

  void DestroyPool();  // 释放所有连接

 private:
  SqlConnPool();
  ~SqlConnPool();

  SqlConnPool(const SqlConnPool &);
  SqlConnPool(const SqlConnPool &&);
  SqlConnPool &operator=(const SqlConnPool &);
  SqlConnPool &operator=(const SqlConnPool &&);

  size_t MAX_CONN_;

  std::queue<MYSQL *> conn_que_;  //数据库连接池, 临界区
  std::mutex mtx_;                // 用于互斥访问临界区资源
  std::unique_ptr<Semaphore> psem_;  // 信号量(指针), 用于约束使用连接池的线程数量
};

class SqlConnRAII {
 public:
  /* 这里设计为指向sql指针的指针、也可以采用下面的指针的引用，效果一致 */
  SqlConnRAII(MYSQL **conn, SqlConnPool *pool);
  // SqlConnRAII(MYSQL *&conn, SqlConnPool *pool);
  ~SqlConnRAII();

 private:
  MYSQL *conn_;
  SqlConnPool *pool_;
};

}  // namespace webserver

#endif