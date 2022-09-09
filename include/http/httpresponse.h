/*
 * @Author       : mark
 * @Date         : 2020-06-25
 * @copyleft Apache 2.0
 * http部分暂时使用此处的代码
 */
#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <fcntl.h>     // open
#include <sys/mman.h>  // mmap, munmap
#include <sys/stat.h>  // stat
#include <unistd.h>    // close

#include <unordered_map>

#include "base/stringbuffer.h"
#include "utils/logger.h"

namespace webserver {

class HttpResponse {
 public:
  HttpResponse();
  ~HttpResponse();

  void Init(const std::string& srcDir, std::string& path,
            bool isKeepAlive = false, int code = -1);
  void MakeResponse(StringBuffer& buff);
  void UnmapFile();
  char* File();
  size_t FileLen() const;
  void ErrorContent(StringBuffer& buff, std::string message);
  int Code() const { return code_; }

 private:
  void AddStateLine_(StringBuffer& buff);
  void AddHeader_(StringBuffer& buff);
  void AddContent_(StringBuffer& buff);

  void ErrorHtml_();
  std::string GetFileType_();

  int code_;
  bool isKeepAlive_;

  std::string path_;
  std::string srcDir_;

  char* mmFile_;
  struct stat mmFileStat_;

  static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
  static const std::unordered_map<int, std::string> CODE_STATUS;
  static const std::unordered_map<int, std::string> CODE_PATH;
};

}  // namespace webserver

#endif  // HTTP_RESPONSE_H