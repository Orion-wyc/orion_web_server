/*
 * @Author       : Orion
 * @Date         : 2022-09-2
 * @copyleft Apache 2.0
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>  // fcntl()
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>  // close()

#include <string>
#include <unordered_map>

#include "base/epoller.h"
#include "base/timer.h"
#include "http/httpconnection.h"
#include "pool/sqlconnpool.h"
#include "pool/threadpool.h"
#include "utils/logger.h"

namespace webserver {

class WebServer {
 public:
  WebServer(int port, int trigMode, int timeoutMS, bool OptLinger, int sqlPort,
            const char* sqlUser, const char* sqlPwd, const char* dbName,
            int connPoolNum, int threadNum, bool openLog, int logLevel,
            int logQueSize);

  ~WebServer();
  void Start();

 private:
  // 最大的 文件描述符(File Descriptor)数量
  static const int MAX_FD = 65536;

  int port_;          // server端口
  bool open_linger_;  // 是否开启"优雅退出"
  int timeout_ms_;    // 超时单位（毫秒ms）
  bool closed_;       // 标记服务器是否处于"关闭"状态
  int listen_fd_;     // server进程的文件描述符（一个整数）

  std::string src_dir_;  // server的源路径
  std::string log_dir_;  // log的存放目录

  uint32_t listen_event_;
  uint32_t conn_event_;

  std::unique_ptr<MinHeapTimer> timer_;
  std::unique_ptr<ThreadPool> threadpool_;
  std::unique_ptr<Epoller> epoller_;
  std::unordered_map<int, HttpConn> users_;  // 映射sockfd和http连接之间的关系

  // 初始化socket
  bool InitSocket_();
  // 初始化epoll的边缘触发/水平触发
  void InitEventMode_(int trigMode);
  // 当收到连接请求时，添加客户端
  void AddClient_(int fd, sockaddr_in addr);

  void DealListen_();
  void DealWrite_(HttpConn* client);
  void DealRead_(HttpConn* client);

  void SendError_(int fd, const char* info);
  void ExtentTime_(HttpConn* client);
  void CloseConn_(HttpConn* client);

  void OnRead_(HttpConn* client);
  void OnWrite_(HttpConn* client);
  void OnProcess(HttpConn* client);

  static int SetFdNonblock(int fd);
};

}  // namespace webserver

#endif  // WSERVER_H