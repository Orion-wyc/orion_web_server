/*
 * @Author       : mark
 * @Date         : 2020-06-15
 * @copyleft Apache 2.0
 * http部分暂时使用此处的代码
 */

#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <arpa/inet.h>  // sockaddr_in
#include <errno.h>
#include <stdlib.h>  // atoi()
#include <sys/types.h>
#include <sys/uio.h>  // readv/writev

#include <string>

#include "base/stringbuffer.h"
#include "http/httprequest.h"
#include "http/httpresponse.h"
#include "pool/sqlconnpool.h"
#include "utils/logger.h"

namespace webserver {

class HttpConn {
 public:
  HttpConn();

  ~HttpConn();

  void init(int sockFd, const sockaddr_in& addr);

  ssize_t read(int* saveErrno);

  ssize_t write(int* saveErrno);

  void Close();

  int GetFd() const;

  int GetPort() const;

  const char* GetIP() const;

  sockaddr_in GetAddr() const;

  bool process();

  int ToWriteBytes() { return iov_[0].iov_len + iov_[1].iov_len; }

  bool IsKeepAlive() const { return request_.IsKeepAlive(); }

  /* 下面三静态成员变量在WebServer的构造函数中初始化 */
  static bool isET;
  // static const char* srcDir;
  static std::string srcDir;
  static std::atomic<int> userCount;

 private:
  int fd_;
  struct sockaddr_in addr_;

  bool isClose_;

  int iovCnt_;
  struct iovec iov_[2];

  StringBuffer readBuff_;   // 读缓冲区
  StringBuffer writeBuff_;  // 写缓冲区

  HttpRequest request_;
  HttpResponse response_;
};

}  // namespace webserver

#endif  // HTTP_CONN_H