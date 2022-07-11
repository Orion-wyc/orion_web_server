/*
 * @Author       : Orion
 * @Date         : 2022-07-10
 * @copyleft Apache 2.0
 */

#include "pool/threadpool.h"
#include "utils/logger.h"

void ThreadLogTask(int i, int cnt) {
  std::thread::id tid = std::this_thread::get_id();
  for (int j = 0; j < 10000; j++) {
    LOG_BASE(i, "PID:[%04u]======= %05d ========= ", tid, cnt++);
  }
}

void TestThreadPool() {
  webserver::Logger::Instance()->Initialize("./__logthreadpool", 0, 4096);
  webserver::ThreadPool threadpool(12);
  for (int i = 0; i < 16; i++) {
    threadpool.AddTask(std::bind(ThreadLogTask, i % 4, i * 10000));
  }
  getchar();
}

int main() {
  TestThreadPool();
  std::cout << "Test ThreadPool Completed" << std::endl;
  return 0;
}