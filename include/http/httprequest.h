/*
 * @Author       : mark
 * @Date         : 2020-06-25
 * @copyleft Apache 2.0
 * http部分暂时使用此处的代码
 */
#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <errno.h>
#include <mysql/mysql.h>  //mysql

#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "base/stringbuffer.h"
#include "pool/sqlconnpool.h"
#include "utils/logger.h"

namespace webserver {

class HttpRequest {
 public:
  enum PARSE_STATE {
    REQUEST_LINE,
    HEADERS,
    BODY,
    FINISH,
  };

  enum HTTP_CODE {
    NO_REQUEST = 0,
    GET_REQUEST,
    BAD_REQUEST,
    NO_RESOURSE,
    FORBIDDENT_REQUEST,
    FILE_REQUEST,
    INTERNAL_ERROR,
    CLOSED_CONNECTION,
  };

  HttpRequest() { Init(); }
  ~HttpRequest() = default;

  void Init();
  bool parse(StringBuffer& buff);

  std::string path() const;
  std::string& path();
  std::string method() const;
  std::string version() const;
  std::string GetPost(const std::string& key) const;
  std::string GetPost(const char* key) const;

  bool IsKeepAlive() const;

  /*
  todo
  void HttpConn::ParseFormData() {}
  void HttpConn::ParseJson() {}
  */

 private:
  /* 以下三个函数用于解析http报文，先处理 RequestLine，若处理成功将 PARSE_STATE
     从 REQUEST_LINE 转为 HEADERS。headers解析成功则将 PARSE_STATE 转为 BODY, 
     继续处理请求主体body。http报文处理完毕的标志是FINISH。*/
    
  bool ParseRequestLine_(const std::string& line);
  void ParseHeader_(const std::string& line);
  void ParseBody_(const std::string& line);

  void ParsePath_();
  void ParsePost_();
  void ParseFromUrlencoded_();

  static bool UserVerify(const std::string& name, const std::string& pwd,
                         bool isLogin);

  PARSE_STATE state_;
  std::string method_, path_, version_, body_;
  std::unordered_map<std::string, std::string> header_;
  std::unordered_map<std::string, std::string> post_;

  static const std::unordered_set<std::string> DEFAULT_HTML;
  static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
  static int ConverHex(char ch);
};

}  // namespace webserver

#endif  // HTTP_REQUEST_H