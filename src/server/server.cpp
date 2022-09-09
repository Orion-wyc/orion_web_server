/*
 * @Author       : Orion
 * @Date         : 2022-09-2
 * @copyleft Apache 2.0
 */

#include "server/server.h"

namespace webserver {

WebServer::WebServer(int port, int trigMode, int timeoutMS, bool OptLinger,
                     int sqlPort, const char* sqlUser, const char* sqlPwd,
                     const char* dbName, int connPoolNum, int threadNum,
                     bool openLog, int logLevel, int logQueSize)
    : port_(port),
      open_linger_(OptLinger),
      timeout_ms_(timeoutMS),
      closed_(false),
      timer_(new MinHeapTimer()),
      threadpool_(new ThreadPool(threadNum)),
      epoller_(new Epoller()) {
  /* 获取当前工作路径，检测路径是否未NULL */
  std::string base_dir(getcwd(nullptr, 256));
  assert(!base_dir.empty());
  // strncat(srcDir_, "/resources/", 16);
  src_dir_ = base_dir + "/website/";
  log_dir_ = base_dir + "/logs/";

  /*HttpConn三个静态成员变量的初始化*/
  HttpConn::userCount = 0;
  HttpConn::srcDir = src_dir_;

  /* 单例模式:SqlConnPool统一管理数据库的连接 */
  SqlConnPool::Instance()->Initialize("localhost", sqlPort, sqlUser, sqlPwd,
                                      dbName, connPoolNum);

  /* -m，listenfd和connfd的模式组合, daima
   * 0，表示使用LT + LT
   * 1，表示使用LT + ET
   * 2，表示使用ET + LT
   * 3，表示使用ET + ET */
  InitEventMode_(trigMode);

  /* 创建listenfd */
  if (!InitSocket_()) {
    closed_ = true;
  }

  /* 是否开启日志, 启用日志则进行初始化 */
  if (openLog) {
    /* 日志记录模块Log也适用单例模式 */
    Logger::Instance()->Initialize(log_dir_, logLevel, logQueSize);

    if (closed_) {
      LOG_ERROR("Server init error!");
    } else {
      LOG_INFO("Server init!");
      LOG_INFO("Port:%d, OpenLinger: %s", port_, OptLinger ? "true" : "false");
      LOG_INFO("listenFd_ value: %d", listen_fd_);
      LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
               (listen_event_ & EPOLLET ? "ET" : "LT"),
               (conn_event_ & EPOLLET ? "ET" : "LT"));
      LOG_INFO("LogSys level: %d", logLevel);
      LOG_INFO("srcDir: %s", HttpConn::srcDir.c_str());
      LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum,
               threadNum);
    }
  }
}

/* 析构函数的操作：关闭listenFd、标记server关闭状态、释放目录、释放mysql连接对象
 * 以ctrl+c方式退出是无法记录析构函数内的日志 */
WebServer::~WebServer() {
  close(listen_fd_);
  closed_ = true;
}

/* 初始化event通知模式，默认是3(ET + ET) */
void WebServer::InitEventMode_(int trigMode) {
  listen_event_ = EPOLLRDHUP;
  conn_event_ = EPOLLONESHOT | EPOLLRDHUP;
  switch (trigMode) {
    case 0:
      break;
    case 1:
      conn_event_ |= EPOLLET;
      break;
    case 2:
      listen_event_ |= EPOLLET;
      break;
    case 3:
      listen_event_ |= EPOLLET;
      conn_event_ |= EPOLLET;
      break;
    default:
      listen_event_ |= EPOLLET;
      conn_event_ |= EPOLLET;
      break;
  }

  /* LT 和 ET的读取方式不同，ET模式下，一次通知需要读取缓冲区全部数据 */
  HttpConn::isET = (conn_event_ & EPOLLET);
}

void WebServer::Start() {
  int timeMS = -1;  // epoll wait timeout == -1 无事件将阻塞
  if (!closed_) {
    LOG_INFO("Server start!");
  }
  while (!closed_) {
    if (timeout_ms_ > 0) {
      timeMS = timer_->GetNextTick();
    }

    /* eventCnt is the number of triggered events returned in "events" buffer */
    int eventCnt = epoller_->EpollWait(timeMS);
    for (int i = 0; i < eventCnt; i++) {
      // 处理事件
      int fd = epoller_->GetEventFd(i);
      uint32_t events = epoller_->GetEpollEvents(i);

      if (fd == listen_fd_) {
        /* 监听socket */
        DealListen_();
      } else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
        /* 关闭连接 */
        assert(users_.count(fd) > 0);
        CloseConn_(&users_[fd]);
      } else if (events & EPOLLIN) {
        /* 读取 */
        assert(users_.count(fd) > 0);
        DealRead_(&users_[fd]);
      } else if (events & EPOLLOUT) {
        assert(users_.count(fd) > 0);
        DealWrite_(&users_[fd]);
      } else {
        LOG_ERROR("Unexpected event");
      }
    }
  }
}

/* 如果服务器端出现错误，则返回info给客户端并关闭当前连接*/
void WebServer::SendError_(int fd, const char* info) {
  assert(fd > 0);
  int ret = send(fd, info, strlen(info), 0);
  if (ret < 0) {
    LOG_WARN("send error to client[%d] error!", fd);
  }
  close(fd);
}

/*服务端正常关闭与某个client的连接*/
void WebServer::CloseConn_(HttpConn* client) {
  assert(client);
  LOG_INFO("Client[%d, %s:%d] quit!", client->GetFd(),
           ConvertIP(client->GetAddr().sin_addr.s_addr).c_str(),
           client->GetAddr().sin_port);
  epoller_->EpollRemove(client->GetFd());
  client->Close();
}

/*服务端新增一个连接*/
void WebServer::AddClient_(int fd, sockaddr_in addr) {
  assert(fd > 0);
  users_[fd].init(fd, addr);

  if (timeout_ms_ > 0) {
    /* 这里的bind用作断开连接的回调函数，比较有趣，传递参数时需要加上this */
    timer_->AddItem(fd, timeout_ms_,
                    std::bind(&WebServer::CloseConn_, this, &users_[fd]));
  }

  epoller_->EpollAdd(fd, EPOLLIN | conn_event_);
  
  /* 设置fd为非阻塞clientfd*/
  SetFdNonblock(fd);

  LOG_INFO("Client[%d, %s:%d] connected!", users_[fd].GetFd(),
           ConvertIP(users_[fd].GetAddr().sin_addr.s_addr).c_str(),
           users_[fd].GetAddr().sin_port);
}

void WebServer::DealListen_() {
  struct sockaddr_in addr;       // 此处表示一个Internet socket address
  socklen_t len = sizeof(addr);  // 获取地址长度，地址内存在padding
  do {
    /* 提取挂起队列的第一个连接请求，创建一个新的连接套接字并返回其fd */
    int fd = accept(listen_fd_, (struct sockaddr*)&addr, &len);
    if (fd <= 0) {
      /* 错误返回, socket为nonblock，队列空则返回EWOULDBLOCK （EAGAIN 11） */
      LOG_ERROR("%s: errno is: %d (%s)", "accept error", errno,
                strerror(errno));
      return;
    } else if (HttpConn::userCount >= MAX_FD) {
      /* 超出最大连接数 */
      SendError_(fd, "Server busy!");
      LOG_WARN("Clients is full!");
      return;
    }
    /* 正常fd则添加新的客户端信息 */
    AddClient_(fd, addr);
  } while (listen_event_ & EPOLLET);  // EPOLLET模式继续循环
}

void WebServer::DealRead_(HttpConn* client) {
  assert(client);
  ExtentTime_(client);
  threadpool_->AddTask(std::bind(&WebServer::OnRead_, this, client));
}

void WebServer::DealWrite_(HttpConn* client) {
  assert(client);
  ExtentTime_(client);
  threadpool_->AddTask(std::bind(&WebServer::OnWrite_, this, client));
}

void WebServer::ExtentTime_(HttpConn* client) {
  assert(client);
  if (timeout_ms_ > 0) {
    timer_->UpdateItem(client->GetFd(), timeout_ms_);
  }
}

void WebServer::OnRead_(HttpConn* client) {
  assert(client);
  int ret = -1;
  int readErrno = 0;
  ret = client->read(&readErrno);
  if (ret <= 0 && readErrno != EAGAIN) {
    CloseConn_(client);
    return;
  }
  /* 先从clientfd读取报文，然后调用HttpConn::process处理请求/响应 */
  OnProcess(client);
}

void WebServer::OnProcess(HttpConn* client) {
  if (client->process()) {
    epoller_->EpollModify(client->GetFd(), conn_event_ | EPOLLOUT);
  } else {
    epoller_->EpollModify(client->GetFd(), conn_event_ | EPOLLIN);
  }
}

void WebServer::OnWrite_(HttpConn* client) {
  assert(client);
  int ret = -1;
  int writeErrno = 0;
  ret = client->write(&writeErrno);
  if (client->ToWriteBytes() == 0) {
    /* 传输完成 */
    if (client->IsKeepAlive()) {
      OnProcess(client);
      return;
    }
  } else if (ret < 0) {
    if (writeErrno == EAGAIN) {
      /* 继续传输 */
      epoller_->EpollModify(client->GetFd(), conn_event_ | EPOLLOUT);
      return;
    }
  }
  CloseConn_(client);
}

/* Create listenFd */
bool WebServer::InitSocket_() {
  int ret;
  /* 监听socket的TCP/IP的IPV4 socket地址 */
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));

  if (port_ > 65535 || port_ < 1024) {
    LOG_ERROR("Port:%d error!", port_);
    return false;
  }
  /* socket连接所属的协议族（默认IP protocol family）*/
  addr.sin_family = AF_INET;
  /* IP地址, INADDR_ANY：将套接字绑定到addr所有可用的接口 */
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  /* 监听端口 */
  addr.sin_port = htons(port_);

  /* 优雅关闭: 当socket关闭时,内核将拖延一段时间直到所剩数据发送完毕或超时
   * Linger意思是"留存" */
  struct linger optLinger = {0};
  if (open_linger_) {
    optLinger.l_onoff = 1;
    optLinger.l_linger = 1;
  }

  /* 创建监听socket文件描述符
   * socket在AF_INET上创建SOCK_STREAM类型的socket,0表示协议自动选择*/
  listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd_ < 0) {
    LOG_ERROR("Create socket error!", port_);
    return false;
  }

  /* SO_LINGER 允许端口被重复使用 */
  ret = setsockopt(listen_fd_, SOL_SOCKET, SO_LINGER, &optLinger,
                   sizeof(optLinger));
  if (ret < 0) {
    close(listen_fd_);
    LOG_ERROR("Init linger error!", port_);
    return false;
  }

  int optval = 1;
  /* 端口复用 */
  /* 只有最后一个套接字会正常接收数据。 */
  ret = setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval,
                   sizeof(int));
  if (ret == -1) {
    LOG_ERROR("set socket setsockopt error!");
    close(listen_fd_);
    return false;
  }

  /* 绑定socket和它的地址addr */
  ret = bind(listen_fd_, (struct sockaddr*)&addr, sizeof(addr));
  if (ret < 0) {
    LOG_ERROR("Bind Port:%d error!", port_);
    close(listen_fd_);
    return false;
  }

  /* 在这些客户连接被accept()之前, 创建监听队列以存放待处理的客户连接 */
  ret = listen(listen_fd_, 6);
  if (ret < 0) {
    LOG_ERROR("Listen port:%d error!", port_);
    close(listen_fd_);
    return false;
  }

  ret = epoller_->EpollAdd(listen_fd_, listen_event_ | EPOLLIN);
  if (ret == 0) {
    LOG_ERROR("Add listen error!");
    close(listen_fd_);
    return false;
  }

  /* 设置非阻塞listenfd*/
  SetFdNonblock(listen_fd_);

  LOG_INFO("Server port:%d", port_);

  return true;
}

int WebServer::SetFdNonblock(int fd) {
  assert(fd > 0);
  /* F_GETFD: 首先获取fd的flags,
     F_SETFL: 然后将其设置为O_NONBLOCK */
  return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

}  // namespace webserver