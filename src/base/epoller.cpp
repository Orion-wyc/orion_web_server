/*
 * @Author       : Orion
 * @Date         : 2022-09-02
 * @copyleft Apache 2.0
 */

#include "base/epoller.h"

#include "utils/logger.h"

namespace webserver {

Epoller::Epoller(size_t max_events)
    : epoll_fd_(epoll_create1(0)), events_(max_events) {
  if (epoll_fd_ == -1 || events_.size() <= 0) {
    LOG_ERROR("Epoller Failed, epoll_fd=%d, events_size=%d", epoll_fd_,
              events_.size());
    exit(0);
  }
}

Epoller::~Epoller() { close(epoll_fd_); }

bool Epoller::EpollAdd(int fd, uint32_t events) {
  if (fd < 0) return false;
  epoll_event ev;
  ev.data.fd = fd;
  ev.events = events;
  return (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == 0);
}

bool Epoller::EpollModify(int fd, uint32_t events) {
  if (fd < 0) return false;
  epoll_event ev;
  ev.data.fd = fd;
  ev.events = events;
  return (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == 0);
}

bool Epoller::EpollRemove(int fd) {
  if (fd < 0) return false;
  //
  epoll_event ev;
  ev.data.fd = fd;
  return (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &ev) == 0);
}

int Epoller::EpollWait(int timeout) {
  return epoll_wait(epoll_fd_, events_.data(), static_cast<int>(events_.size()),
                    timeout);
}

int Epoller::GetEventFd(size_t idx) const { return events_.at(idx).data.fd; }

uint32_t Epoller::GetEpollEvents(size_t idx) const {
  return events_.at(idx).events;
}

int Epoller::GetEpollFd() const { return epoll_fd_; }

}  // namespace webserver