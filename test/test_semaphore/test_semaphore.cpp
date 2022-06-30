/* 编译指令 g++ -o test_semaphore  test_semaphore.cpp ../src/base/semaphore.cpp
 *  -I../include/ -lpthread
 *
 * 本测试样基于信号量Semaphore实现了基于阻塞队列的生产者-消费者模式, 
 * 但是该模式下没有实现如何控制队列的容量capacity, 相关实现可以参考
 * 阻塞队列BlockQueue.h, 它使用一个互斥量和两个条件变量实现生产者-消
 * 费者模式。
 */

/*
// 简单地测试 Semophore

#include <iostream>
#include <thread>

#include "base/semaphore.h"

webserver::Semaphore sema(0);

void func1() {
  // do something
  std::this_thread::sleep_for(std::chrono::seconds(2));
  std::cout << "func1" << std::endl;
  sema.Release();
}

void func2() {
  sema.Acquire();
  // do something
  std::cout << "func2" << std::endl;
}

int main() {
  std::thread thread1(func1);
  std::thread thread2(func2);
  if (thread1.joinable()) thread1.join();
  if (thread2.joinable()) thread2.join();
}
*/

#include <unistd.h>

#include <iostream>
#include <list>
#include <mutex>
#include <random>
#include <thread>

#include "base/semaphore.h"

using namespace std;

class Goods {
 public:
  explicit Goods(int ii) : id(ii) {}
  int id;
};  //产品

webserver::Semaphore *g_sem;

list<Goods> g_goods;  //商品货架

mutex g_mutex;

void producer(int a = 0, int b = 10) {
  default_random_engine e(static_cast<unsigned long>(time(0)));
  uniform_int_distribution<unsigned> u(a, b);
  int i = 0;
  while (i++ < 5) {
    //休眠一段随机时间,代表生产过程
    sleep(1);
    //生产
    g_mutex.lock();
    Goods good(u(e));
    g_goods.emplace_back(good);
    cout << "生产产品:" << good.id << endl;
    cout << "产品数量:" << g_goods.size() << endl;
    g_mutex.unlock();
    //唤醒一个阻塞的消费者
    g_sem->Release();
  }
}

void costumer() {
  int i = 0;
  while (i++ < 10) {
    g_sem->Acquire();  //有资源会立即返回,没有资源则会等待
    //消费
    g_mutex.lock();
    Goods good = g_goods.front();
    g_goods.pop_front();
    g_mutex.unlock();

    cout << "消费产品:" << good.id << endl;
    sleep(2);
  }
}

int main() {
  g_sem = new webserver::Semaphore(0);
  thread producer_t(producer, 0, 5);
  thread producer_t2(producer, 5, 10);
  thread costumer_t(costumer);
  producer_t.join();
  producer_t2.join();
  costumer_t.join();
  delete g_sem;
  return 0;
}
