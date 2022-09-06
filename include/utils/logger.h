/*
 * @Author       : Orion
 * @Date         : 2022-06-30
 * @copyleft Apache 2.0
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <sys/stat.h>  //mkdir

#include <chrono>
#include <cstdarg>  // vastart va_end
#include <cstdio>
#include <ctime>
#include <string>
#include <vector>

#include "base/blockqueue.h"
#include "base/stringbuffer.h"
#include "utils/converter.h"

namespace webserver {

class Logger {
 public:
  static Logger *Instance();

  void Initialize(const std::string &log_dir, int level = 1,
                  size_t que_size = 1024);

  static void FlushLogThread();

  void WriteLog(int level, const char *fmt, ...);
  void FlushToFile();

  void SetLevel(int level);
  int GetLevel() const;
  bool Closed();

 private:
  static const size_t FILE_NAME_LEN;
  static const size_t MAX_CONTENT_LEN;
  static const std::string level_strs_[4];
  /* 日志文件存储目录 */
  std::string log_dir_;
  /* 日志文件名(完整路径) */
  std::string filename_;
  /* 日志等级[0,1,2,3] ，数字越小日志，信息越丰富 */
  int level_;
  /* 是否使用异步日志，默认为同步(false)，que_size > 0 时自动设置为true */
  bool async_;
  /* 日志状态，默认为关闭，文件可写入时为true */
  bool closed_;

  FILE *fp_;
  StringBuffer buff_;
  std::unique_ptr<BlockQueue<std::string>> que_;
  std::unique_ptr<std::thread> flush_thread_;
  std::mutex mtx_;

  Logger();
  ~Logger();
  void AsyncWrite_();
  void WriteLogLevel(int level);
  void WriteDatetime();
};

}  // namespace webserver

#define filename(x) strrchr(x, '/') ? strrchr(x, '/') + 1 : x

#define LOG_BASE(level, format, ...)                                  \
  do {                                                                \
    webserver::Logger *log = webserver::Logger::Instance();           \
    if (!log->Closed() && log->GetLevel() <= level) {                 \
      std::string fmt = "[%s:%d] " + std::string(format);             \
      log->WriteLog(level, fmt.c_str(), filename(__FILE__), __LINE__, \
                    ##__VA_ARGS__);                                   \
      log->FlushToFile();                                             \
    }                                                                 \
  } while (0);

#define LOG_DEBUG(format, ...)         \
  do {                                 \
    LOG_BASE(0, format, ##__VA_ARGS__) \
  } while (0);

#define LOG_INFO(format, ...)          \
  do {                                 \
    LOG_BASE(1, format, ##__VA_ARGS__) \
  } while (0);

#define LOG_WARN(format, ...)          \
  do {                                 \
    LOG_BASE(2, format, ##__VA_ARGS__) \
  } while (0);

#define LOG_ERROR(format, ...)         \
  do {                                 \
    LOG_BASE(3, format, ##__VA_ARGS__) \
  } while (0);

#endif
