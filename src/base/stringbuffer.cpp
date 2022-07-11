/*
 * @Author       : Orion
 * @Date         : 2022-07-06
 * @copyleft Apache 2.0
 */

#include "base/stringbuffer.h"

namespace webserver {

StringBuffer::StringBuffer(size_t init_buff_size)
    : buffer_(init_buff_size), read_pos_(0), write_pos_(0) {}

size_t StringBuffer::ReadableBytes() const { return write_pos_ - read_pos_; }

size_t StringBuffer::PreWritableBytes() const { return read_pos_; }

size_t StringBuffer::PostWritableBytes() const {
  return buffer_.size() - write_pos_;
}

size_t StringBuffer::Capacity() const { return buffer_.size(); }

void StringBuffer::EnsureWritable(size_t len) {
  if (PostWritableBytes() < len) {
    ManageSpace_(len);
  }
  assert(PostWritableBytes() >= len);
}

void StringBuffer::CompleteWriting(size_t len) { write_pos_ += len; }

void StringBuffer::Clear() {
  bzero(buffer_.data(), buffer_.size());
  write_pos_ = 0;
  read_pos_ = 0;
}

std::string StringBuffer::RetrieveAllToStr() {
  std::string str(ReadBeginPtr(), ReadableBytes());
  Clear();
  return str;
}

const char *StringBuffer::ReadBeginPtr() const {
  return BeginPtr_() + read_pos_;
}

char *StringBuffer::WriteBeginPtr() { return BeginPtr_() + write_pos_; }

const char *StringBuffer::WriteBeginPtr() const {
  return BeginPtr_() + write_pos_;
}

void StringBuffer::Append(const char *data, size_t len) {
  assert(data);
  /* 此处单独检查 */
  if (len <= 0) return;
  len = len > strlen(data) ? strlen(data) : len;
  EnsureWritable(len);
  std::copy(data, data + len, WriteBeginPtr());
  CompleteWriting(len);
}

void StringBuffer::Append(const std::string &str) {
  Append(str.data(), str.size());
}

void StringBuffer::Append(const StringBuffer &buff) {
  Append(buff.ReadBeginPtr(), buff.ReadableBytes());
}

/* ---------------------- 私有方法 ---------------------- */

char *StringBuffer::BeginPtr_() { return buffer_.data(); }

const char *StringBuffer::BeginPtr_() const { return buffer_.data(); }

void StringBuffer::ManageSpace_(size_t len) {
  if (PreWritableBytes() + PostWritableBytes() < len) {
    buffer_.resize(write_pos_ + len + 1);
  } else {
    size_t read_size = ReadableBytes();
    std::copy(BeginPtr_() + read_pos_, BeginPtr_() + write_pos_, BeginPtr_());
    read_pos_ = 0;
    write_pos_ = read_pos_ + read_size;
    assert(read_size == ReadableBytes());
  }
}

}  // namespace webserver