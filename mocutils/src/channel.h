#ifndef __MOC_CHANNEL_H
#define __MOC_CHANNEL_H

#include <mutex>
#include <utility>
#include <queue>
#include <memory>
#include "./semaphore.h"

/**
 * channel 类的设计和功能可以被视为一种管道机制
 * 在代码中，channel 类及其派生类 nbchannel 和 bchannel 实现了数据的传输和同步机制
 * 
 * channel 允许数据在生产者和消费者之间流动。
 * 生产者可以通过 << 操作符发送数据，消费者可以通过 >> 操作符接收数据
 * 
 * nbchannel 类实现了非阻塞的通道，可以在没有数据时返回，类似于非阻塞管道
 * bchannel 类实现了阻塞的通道，在没有数据时会阻塞，直到数据可用，这与阻塞管道的行为类似
 * 
 * channel 类中的实现使用了 std::mutex 和自定义的 semaphore 来控制数据的访问和同步
 */

namespace moc {

template <typename T, int _cap = 0>
class channel {
public:
  int size, cap;
  channel(): size(0), cap(_cap) {}
  virtual void operator<<(T _value) = 0;
  virtual void operator>>(T &_value) = 0;
};

//不阻塞（Non-Blocking）通道的实现
template <typename T>
class nbchannel: public channel<T> {
  std::queue<std::pair<std::mutex*, T>*> read, write;
  std::mutex clock;
 public:
  nbchannel(): channel<T>() {}
  void operator<<(T _value);//写入
  void operator>>(T &_value);//读取
};

// write
template <typename T>
void nbchannel<T>::operator<<(T _value) {
  std::pair<std::mutex*, T> *local = nullptr;
  clock.lock();
  if (read.size()) {
    read.front()->second = _value;
    read.front()->first->unlock();
    read.pop();
  } else {
    local = new std::pair<std::mutex*, T>{new std::mutex, _value};
    local->first->lock();
    write.push(local);
  }
  clock.unlock();
  if (local == nullptr) return;
  local->first->lock();
  delete local->first;
  delete local;
}

// read
template <typename T>
void nbchannel<T>::operator>>(T &_value) {
  std::pair<std::mutex*, T> *local = nullptr;
  clock.lock();
  if (write.size()) {
    _value = write.front()->second;
    write.front()->first->unlock();
    write.pop();
  } else {
    local = new std::pair<std::mutex*, T>{new std::mutex, _value};
    local->first->lock();
    read.push(local);
  }
  clock.unlock();
  if (local == nullptr) return;
  local->first->lock();
  _value = local->second;
  delete local->first;
  delete local;
}

//bchannel: 阻塞（Blocking）通道的实现，带有容量限制
template<typename T, int _cap>
class bchannel: public channel<T, _cap> {
  std::queue<T> content;
  std::mutex clock;
  semaphore<_cap> full, empty;
public:
  bchannel(): channel<T, _cap>(), empty(semaphore<_cap>(_cap)) {}
  void operator<<(T _value);
  void operator>>(T &_value);
};

// write
template<typename T, int _cap>
void bchannel<T, _cap>::operator<<(T _value) {
  empty.acquire();
  clock.lock();
  this->size += 1;
  content.push(_value);
  clock.unlock();
  full.release();
}

// read
template<typename T, int _cap>
void bchannel<T, _cap>::operator>>(T &_value) {
  full.acquire();
  clock.lock();
  this->size -= 1;
  _value = content.front();
  content.pop();
  clock.unlock();
  empty.release();
}

template <typename T>
nbchannel<T> make_channel() {
  return nbchannel<T>();
}

template <typename T, int _cap>
bchannel<T, _cap> make_channel() {
  return bchannel<T, _cap>();
}

template <typename T>
nbchannel<T>* makeptr_channel() {
  return new nbchannel<T>;
}

template <typename T, int _cap>
bchannel<T, _cap>* makeptr_channel() {
  return new bchannel<T, _cap>;
}

#include "./macro.h"

};  // namespace moc

#endif