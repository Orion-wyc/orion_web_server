/*
 * @Author       : Orion
 * @Date         : 2022-07-05
 * @copyleft Apache 2.0
 */

#ifndef STRINGBUFFER_H_
#define STRINGBUFFER_H_

#include <sys/uio.h>  //readv
#include <unistd.h>   // write

#include <atomic>
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace webserver {
/*  缓冲区示意图
 *  buffer_ --> [BeginPtr,..., ReadBeginPtr, ..., WriteBeginPtr,...EndPtr];
 *    EndPtr(最大下标的后一个位置，不实现)
 *    ReadBeginPtr (Peek) = BeginPtr + read_pos_
 *    WriteBeginPtr = BeginPtr + write_pos_
 *  其中，可写的区间包括两部分：
 *    前部缓冲区PreWritable --> [BeginPtr, ReadBeginPtr)
 *    尾部缓冲区PostWritable --> [WriteBeginPtr, EndPtr)
 *  可读数据区间为：
 *    [ReadBeginPtr, ..., WriteBeginPtr) 长度为 writePos_ - readPos_;
 */
class StringBuffer {
 public:
  StringBuffer(size_t init_buff_size = 1024);
  ~StringBuffer() = default;

  /* 获取可读区间字节数，前、后可写区间字节数 */
  size_t ReadableBytes() const;
  size_t PreWritableBytes() const;
  size_t PostWritableBytes() const;
  size_t Capacity() const;

  /* 判断尾部缓冲区长度是否足够，不足需要调整缓冲区大小或移动数据 */
  void EnsureWritable(size_t len);

  /* 完成缓冲区写入后，更新可写区间起始位置write_pos_ */
  void CompleteWriting(size_t len);

  /* 将缓冲区所有字节全部置为0 */
  void Clear();

  /* 将Readable缓冲区的字符转化为std::string对象, 清空整个缓冲区 */
  std::string RetrieveAllToStr();

  /* http报文解析中需要使用的函数 */
  void Retrieve(size_t len);
  void RetrieveUntil(const char *end);
  void RetrieveAll();

  /* 获取Readable/Writable缓冲区的首部指针,
     其中读指针为const类型，写指针可修改内容，
     在http报文解析过程中，使用std::search
     需要强制转换为 const char* 类型 */
  const char *ReadBeginPtr() const;
  char *WriteBeginPtr();

  void Append(const char *data, size_t len);
  void Append(const std::string &str);
  void Append(const StringBuffer &buff);

  /* 从fd中读取数据写入buff_ */
  ssize_t ReadFromFd(int fd, int *Errno);
  /* 从buff_中读取数据写入fd */
  ssize_t WriteToFd(int fd, int *Errno);

 private:
  std::vector<char> buffer_;
  std::atomic<size_t> read_pos_;
  std::atomic<size_t> write_pos_;

  char *BeginPtr_();
  const char *BeginPtr_() const;
  void ManageSpace_(size_t len);
};

}  // namespace webserver

#endif