/*
 * @Author       : Orion
 * @Date         : 2022-06-28
 * @copyleft Apache 2.0
 */

#ifndef UNCOPYABLE_H_
#define UNCOPYABLE_H_

namespace webserver {

/* 参考boost::noncopyable实现 */
class Uncopyable {
 protected:
  Uncopyable(){};
  ~Uncopyable(){};

 private:
  Uncopyable(const Uncopyable &obj);
  Uncopyable &operator=(const Uncopyable &obj);
};

}  // namespace webserver

#endif