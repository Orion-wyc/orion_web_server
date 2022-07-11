/*
 * @Author       : Orion
 * @Date         : 2022-07-06
 * @copyleft Apache 2.0
 */

#include <iostream>
#include <string>

#include "base/stringbuffer.h"

int main() {
  webserver::StringBuffer buff(4);
  std::cout << "buffer capacity = " << buff.Capacity() << std::endl;
  const char *s1 = "ab";
  std::string s2 = "wxyz";
  webserver::StringBuffer b2(2);
  b2.Append("ab");
  buff.Append(s1, 4);
  std::cout << "buffer capacity = " << buff.Capacity() << std::endl;
  std::cout << "buffer readable bytes = " << buff.ReadableBytes() << std::endl;
  std::cout << "pre writable bytes = " << buff.PreWritableBytes() << std::endl;
  std::cout << "post writable bytes = " << buff.PostWritableBytes()
            << std::endl;

  buff.Append(s2);
  std::cout << "buffer capacity = " << buff.Capacity() << std::endl;
  std::cout << "buffer readable bytes = " << buff.ReadableBytes() << std::endl;
  std::cout << "pre writable bytes = " << buff.PreWritableBytes() << std::endl;
  std::cout << "post writable bytes = " << buff.PostWritableBytes()
            << std::endl;
  buff.Append(b2);
  std::cout << "buffer capacity = " << buff.Capacity() << std::endl;
  std::cout << "buffer readable bytes = " << buff.ReadableBytes() << std::endl;
  std::cout << "pre writable bytes = " << buff.PreWritableBytes() << std::endl;
  std::cout << "post writable bytes = " << buff.PostWritableBytes()
            << std::endl;

  std::cout << "buffer content = " << buff.RetrieveAllToStr() << std::endl;
  std::cout << "buffer capacity = " << buff.Capacity() << std::endl;
  std::cout << "buffer readable bytes = " << buff.ReadableBytes() << std::endl;
  std::cout << "pre writable bytes = " << buff.PreWritableBytes() << std::endl;
  std::cout << "post writable bytes = " << buff.PostWritableBytes()
            << std::endl;

  return 0;
}