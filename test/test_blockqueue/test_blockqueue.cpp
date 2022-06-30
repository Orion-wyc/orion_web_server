/*
 * @Author       : Orion
 * @Date         : 2022-06-30
 * @copyleft Apache 2.0
 */

#include <cstdio>
#include <iostream>
#include <string>
#include <thread>

#include "base/blockqueue.h"

webserver::BlockQueue<int> block_que_(10);

void produce(int tid) {
  int num = 0;
  for (;;) {
    block_que_.Push(num);
    ++num;
    printf("P %d Produced a Num=%d (Size=%lu)\n", tid, block_que_.Back(),
           block_que_.Size());
    // std::cout << "Produced a Num (Size): " << block_que_.Back() << " "
    //           << block_que_.Size() << std::endl;
    // std::cout << "->Block Queue Size = " << block_que_.Size() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
  }
}

void consume(int tid) {
  for (;;) {
    int num = 0;
    if (block_que_.Pop(num)) {
      // std::cout << "Consume a Num: " << num << std::endl;
      printf("->P %d Consumed a Num=%d (Size=%lu)\n", tid, num,
             block_que_.Size());
    } else {
      std::cout << "->Block Queue is Empty" << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

int main() {
  std::thread producer1(produce, 100);
  std::thread producer2(produce, 200);
  std::thread consumer(consume, 300);
  producer1.join();
  producer2.join();
  consumer.join();
  return 0;
}
