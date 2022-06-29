/*
 * @Author       : Orion
 * @Date         : 2022-06-29
 * @copyleft Apache 2.0
 */

#include "pool/sqlconnpool.h"

#include <cassert>

namespace webserver {

SqlConnPool::SqlConnPool() {}

SqlConnPool *SqlConnPool::Instance() {
  static SqlConnPool instance;
  return &instance;
}

void SqlConnPool::Initialize(const char *host, int port, const char *user,
                             const char *pwd, const char *db_name,
                             int conn_size) {
  assert(conn_size > 0);
  for (int i = 0; i < conn_size; ++i) {
    MYSQL *sql = nullptr;
    /* 如果sql是nullptr, mysql_init会自动创建一个sql对象并初始化,
       同时, mysql_library_init()也被会调用*/
    sql = mysql_init(sql);
    if (!sql) {
      // todo 这里需要补充日志信息
      assert(sql);
    }

    sql = mysql_real_connect(sql, host, user, pwd, db_name, port, nullptr, 0);
    if (!sql) {
      // todo 这里需要补充日志信息
    }
    conn_que_.push(sql);

    /* 初始化信号量, 析构时需要释放psem_空间 */
    MAX_CONN_ = conn_size;
    psem_ = new Semaphore(MAX_CONN_);
  }
}

MYSQL *SqlConnPool::AcquireConn() {
  MYSQL *sql = nullptr;
  if (conn_que_.empty()) {
    // todo 这里需要补充日志信息
    return nullptr;
  }

  /* 信号量计数值减一 */
  psem_->Acquire();
  {
    std::lock_guard<decltype(mtx_)> lock(mtx_);
    sql = conn_que_.front();
    conn_que_.pop();
  }

  return sql;
}

void SqlConnPool::ReleaseConn(MYSQL *sql) {
  assert(sql);
  std::lock_guard<decltype(mtx_)> lock(mtx_);
  conn_que_.push(sql);
  sql = nullptr;
  /* 信号量计数值加一 */
  psem_->Release();
}

size_t SqlConnPool::GetFreeCount() {
  std::lock_guard<decltype(mtx_)> lock(mtx_);
  return conn_que_.size();
}

void SqlConnPool::DestroyPool() {
  std::lock_guard<decltype(mtx_)> lock(mtx_);
  while (!conn_que_.empty()) {
    auto sql = conn_que_.front();
    conn_que_.pop();
    mysql_close(sql);
  }
  mysql_library_end();
}

SqlConnPool::~SqlConnPool() {
  DestroyPool();
  delete psem_;
}

/* SqlConnRALL的定义, 从pool中获取一个sql连接 */
SqlConnRAII::SqlConnRAII(MYSQL **sql, SqlConnPool *pool) {
  assert(pool);
  *sql = pool->AcquireConn();
  conn_ = *sql;
  pool_ = pool;
}

/* 析构时释放构造时获得的sql连接, pool_资源无需释放 */
SqlConnRAII::~SqlConnRAII() {
  if (conn_ != nullptr) {
    pool_->ReleaseConn(conn_);
  }
}

}  // namespace webserver