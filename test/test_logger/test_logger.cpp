/*
 * @Author       : Orion
 * @Date         : 2022-07-10
 * @copyleft Apache 2.0
 */
#include "utils/logger.h"

void TestLogger() {
  int cnt = 0, level = 0;
  webserver::Logger::Instance()->Initialize("./testlog2", level, 4096);
  for (level = 0; level < 4; level++) {
    webserver::Logger::Instance()->SetLevel(level);
    for (int j = 0; j < 1000; j++) {
      for (int i = 0; i < 4; i++) {
        LOG_BASE(i, "%s 222222222 %d ============= ", "Test", cnt++);
      }
    }
  }
}

int main() {
  TestLogger();
  return 0;
}