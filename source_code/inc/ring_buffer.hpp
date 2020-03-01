

#pragma once

#include <etl/array.h>

namespace ring_buf {

// Generic ring buffer class
template <typename T, unsigned int TSize>
class RingBuffer {
 public:
  RingBuffer() :
    max_size(TSize) {}

  void Put(T item);
  T Get();
  void Reset();
  bool IsEmpty() const;
  bool IsFull() const;
  size_t Capacity() const;
  size_t Size() const;

 private:
  etl::array<T, TSize> buf;
  size_t head = 0;
  size_t tail = 0;
  const size_t max_size;
  bool full = false;
};

template <typename T, unsigned int TSize>
void RingBuffer<T, TSize>::Put(T item) {
  buf[head] = item;
  if (full) {
    tail = (tail + 1) % max_size;
  }

  head = (head + 1) % max_size;
  full = (head == tail) ? true : false;
}

template <typename T, unsigned int TSize>
T RingBuffer<T, TSize>::Get() {
  if (IsEmpty()) {
    return T();
  }

  auto val = buf[tail];
  full = false;
  tail = (tail + 1) % max_size;

  return val;
}

template <typename T, unsigned int TSize>
void RingBuffer<T, TSize>::Reset() {
  head = tail;
  full = false;
}

template <typename T, unsigned int TSize>
bool RingBuffer<T, TSize>::IsEmpty() const {
  return (!full && (head == tail)) ? true : false;
}

template <typename T, unsigned int TSize>
bool RingBuffer<T, TSize>::IsFull() const {
  return full;
}

template <typename T, unsigned int TSize>
size_t RingBuffer<T, TSize>::Capacity() const {
  return max_size;
}

template <typename T, unsigned int TSize>
size_t RingBuffer<T, TSize>::Size() const {
  size_t size = max_size;
  if (!full) {
    if (head >= tail) {
      size = head - tail;
    } else {
      size = max_size + head - tail;
    }
  }

  return size;
}

};  // namespace ring_buf
