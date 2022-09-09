/*
 * @Author       : Orion
 * @Date         : 2022-09-2
 * @copyleft Apache 2.0
 */

#include <unistd.h>
#include "server/server.h"

int main() {
  /* 守护进程 后台运行 */
  // daemon(1, 0);

  // todo 改成json配置文件, 修改配置不用重新编译
  webserver::WebServer server(
      /* 服务器配置: 服务器端口 ET模式 timeout(ms) 优雅退出 */
      1317, 3, 6000, false,
      /* Mysql配置: 数据库端口 用户名 密码 数据库名（默认使用user表）*/
      3306, "orion", "orion", "yourdb",
      /* 线程池配置: 连接池数量 线程池数量*/
      2, 6,
      /* 日志配置: 日志开关 日志等级 日志异步队列容量 */
      true, 0, 4096);

  server.Start();

  return 0;
}