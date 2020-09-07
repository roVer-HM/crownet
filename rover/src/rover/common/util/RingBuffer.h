//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#pragma once

#include <vector>

namespace rover {
template <class T>
class RingBuffer {
 public:
  RingBuffer() : _size(10){};
  RingBuffer(int size) : _size(size) { buffer.reserve(size); };
  virtual ~RingBuffer(){};

  void put(T item);
  T getHead();
  T getTail();
  T removeFromHead();
  T removeFromTail();
  bool empty();
  void clear();
  void set(int size);
  // return data. old -> new If reverse == true return data new -> old
  std::vector<T> getData(bool reverse = false);
  int size();

 private:
  void incPointers();

 private:
  int _size;
  int head = 0;
  int tail = 0;
  bool full = false;
  std::vector<T> buffer;
};

template <class T>
int RingBuffer<T>::size() {
  return buffer.size();
}

template <class T>
void RingBuffer<T>::incPointers() {
  if (full) {
    tail = (tail + 1) % _size;
  }

  head = (head + 1) % _size;
  full = head == tail;
}

template <class T>
void RingBuffer<T>::set(int size) {
  clear();
  this->_size = size;
  buffer.reserve(size);
}

template <class T>
bool RingBuffer<T>::empty() {
  return head == tail && !full;
}

template <class T>
T RingBuffer<T>::getHead() {
  return buffer[(head + _size - 1) % _size];
}

template <class T>
T RingBuffer<T>::getTail() {
  return buffer[tail];
}

template <class T>
T RingBuffer<T>::removeFromHead() {
  T ret = buffer[head];
  head = (head + _size - 1) % _size;
  full = false;
  return ret;
}

template <class T>
T RingBuffer<T>::removeFromTail() {
  T ret = buffer[tail];
  tail = (tail + 1) % _size;
  full = false;
  return ret;
}

template <class T>
void RingBuffer<T>::put(T item) {
  buffer[head] = item;
  incPointers();
}

template <class T>
void RingBuffer<T>::clear() {
  buffer.clear();
  head = 0;
  tail = 0;
  full = false;
}

template <class T>
std::vector<T> RingBuffer<T>::getData(bool reverse) {
  std::vector<T> ret;
  int numData = full ? _size : std::abs(head - tail);

  if (reverse) {
    // new --> old
    for (int i = 0; i < numData; i++) {
      ret.push_back(buffer[(head + _size - (1 + i)) % _size]);
    }
  } else {
    // old --> new
    for (int i = 0; i < numData; i++) {
      ret.push_back(buffer[(tail + i) % _size]);
    }
  }
  return ret;
}

}  // namespace rover
