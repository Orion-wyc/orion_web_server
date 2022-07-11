/*
 * @Author       : Orion
 * @Date         : 2022-06-30
 * @copyleft Apache 2.0
 */

#include "utils/logger.h"

namespace webserver {

const size_t Logger::FILE_NAME_LEN = 128;
const size_t Logger::MAX_CONTENT_LEN = 4096;
const std::string Logger::level_strs_[4] = {"[DEBUG] | ", "[INFO]  | ",
                                            "[WARN]  | ", "[ERROR] | "};

Logger *Logger::Instance() {
  static Logger logger;
  return &logger;
}

void Logger::Initialize(const std::string &log_dir, int level,
                        size_t que_size) {
  closed_ = false;
  level_ = level;
  if (que_size >= 1) {
    async_ = true;
    if (!que_) {
      /* 这里的设计还需要重新考量，当前buff是固定大小 */
      std::unique_ptr<BlockQueue<std::string>> new_que(
          new BlockQueue<std::string>(que_size));
      que_ = std::move(new_que);

      std::unique_ptr<std::thread> new_thread(new std::thread(FlushLogThread));
      flush_thread_ = std::move(new_thread);
    }
  }  // else async is false

  /* 配置日志文件路径和名字，目前所有日志写在一个文件中 */
  time_t timer = time(nullptr);
  tm sys_time;
  localtime_r(&timer, &sys_time);

  char filename[FILE_NAME_LEN] = {0};
  snprintf(filename, FILE_NAME_LEN - 1, "%s/%04d_%02d_%02d.log",
           log_dir.c_str(), sys_time.tm_year + 1900, sys_time.tm_mon + 1,
           sys_time.tm_mday);
  filename_ = filename;  // 目前没用

  /* 打开文件 */
  {
    std::lock_guard<decltype(mtx_)> lock(mtx_);
    fp_ = fopen(filename, "a");
    if (fp_ == nullptr) {
      mkdir(log_dir.c_str(), 0777);
      fp_ = fopen(filename, "a");
    }
  }
}

void Logger::FlushLogThread() { Logger::Instance()->AsyncWrite_(); }

void Logger::WriteLog(int level, const char *fmt, ...) {
  /* 读取参数列表，将日志内容写入buff_ */
  va_list vargs;
  {
    std::lock_guard<decltype(mtx_)> lock(mtx_);

    /* 记录行时间和级别也需要加锁 */
    WriteDatetime();
    WriteLogLevel(level);
    
    va_start(vargs, fmt);
    /* 这里日志内容长度不能好过4096字符 */
    int n =
        vsnprintf(buff_.WriteBeginPtr(), buff_.PostWritableBytes(), fmt, vargs);
    va_end(vargs);
    buff_.CompleteWriting(n);
    buff_.Append("\n\0");

    /* 如果阻塞队列未满，则写入队列作为缓冲；否则，直接写入文件 */
    if (async_ && que_ && !que_->Full()) {
      que_->Push(buff_.RetrieveAllToStr());
    } else {
      fputs(buff_.ReadBeginPtr(), fp_);
    }
    /* 一定清空StringBuffer */
    buff_.Clear();
  }
}

void Logger::FlushToFile() {
  /* todo 这里不能清空队列日志 */
  // if (async_) {
  //   que_->Flush();
  // }
  /* 这里原来写了一个死锁 */
  std::lock_guard<decltype(mtx_)> lock(mtx_);
  fflush(fp_);
}

void Logger::SetLevel(int level) { level_ = level; }

int Logger::GetLevel() const { return level_; }

bool Logger::Closed() { return closed_; }

/* -------------------- 私有方法 -------------------- */

Logger::Logger()
    : log_dir_(""),
      filename_(""),
      level_(1),
      async_(false),
      closed_(true),
      fp_(nullptr),
      buff_(MAX_CONTENT_LEN),
      que_(nullptr),
      flush_thread_(nullptr) {}

/* todo 这里析构可能存在问题 */
Logger::~Logger() {
  if (flush_thread_ && flush_thread_->joinable()) {
    while (!que_->Empty()) {
      que_->Flush();
    };
    /* 需要先调用唤醒其它所有阻塞线程，但是目前这里DestroyQueue两次 */
    que_->DestroyQueue();
    flush_thread_->join();
  }

  if (fp_ != nullptr) {
    FlushToFile();
    fclose(fp_);
  }
}

void Logger::AsyncWrite_() {
  std::string str;
  while (que_->Pop(str)) {
    std::lock_guard<decltype(mtx_)> lock(mtx_);
    fputs(str.c_str(), fp_);
  }
}

void Logger::WriteLogLevel(int level) {
  if (level < 0) level = 0;
  if (level >= 4) level = 3;
  buff_.Append(level_strs_[level]);
}

void Logger::WriteDatetime() {
  auto now = std::chrono::system_clock::now();
  long long ms_since_epoch =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          now.time_since_epoch())
          .count();
  time_t sec_since_epoch = time_t(ms_since_epoch / 1000);
  tm time_info;
  localtime_r(&sec_since_epoch, &time_info);
  size_t len = snprintf(buff_.WriteBeginPtr(), 128,
                        "%04d-%02d-%02d %02d:%02d:%02d.%03lld  ",
                        1900 + time_info.tm_year, 1 + time_info.tm_mon,
                        time_info.tm_mday, time_info.tm_hour, time_info.tm_min,
                        time_info.tm_sec, ms_since_epoch % 1000);
  buff_.CompleteWriting(len);
}

}  // namespace webserver