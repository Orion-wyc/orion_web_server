/*
 * @Author       : Orion
 * @Date         : 2022-09-02
 * @copyleft Apache 2.0
 */

#ifndef EPOLLER_H_
#define EPOLLER_H_

#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <vector>

namespace webserver {

class Epoller {
 public:
  explicit Epoller(size_t max_events = 512);
  ~Epoller();
  /* 添加, 修改, 删除fd */
  bool EpollAdd(int fd, uint32_t events);
  bool EpollModify(int fd, uint32_t event);
  bool EpollRemove(int fd);

  /* epoll_wait的简单封装 */
  int EpollWait(int timeout = -1);

  /* 从事件队列中获取一个事件对应的fd, 支持随机访问 */
  int GetEventFd(size_t idx) const;
  /* 从事件队列中获取一个epoll_event事件支持的events, 支持随机访问 */
  uint32_t GetEpollEvents(size_t idx) const;
  /* 获取Epoll实例的fd */
  int GetEpollFd() const;

  /* 禁止拷贝 */
  Epoller(const Epoller &ep) = delete;
  Epoller(const Epoller &&ep) = delete;
  Epoller &operator=(const Epoller &ep) = delete;
  Epoller &operator=(const Epoller &&ep) = delete;

 private:
  int epoll_fd_;
  std::vector<epoll_event> events_;
};

}  // namespace webserver

#endif
