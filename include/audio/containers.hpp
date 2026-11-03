#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <stdexcept>

namespace audio {
template <typename T, std::size_t N> class RingBuffer {
public:
  class RingBufferIterator;
  using iterator = RingBufferIterator;
  using const_iterator = const RingBufferIterator;

  RingBuffer() = default;
  RingBuffer(const RingBuffer &other) = default;
  RingBuffer &operator=(const RingBuffer &other) = default;
  RingBuffer(RingBuffer &&other) = default;
  RingBuffer &operator=(RingBuffer &&other) = default;

  template <typename InputIterator>
  RingBuffer(InputIterator first, InputIterator last) {
    while (first != last) {
      push(*first);
      ++first;
    }
  }

  RingBuffer(std::initializer_list<T> init) {
    for (const auto &item : init) {
      push(item);
    }
  }

  T &at(std::size_t pos) {
    if (pos >= size()) {
      throw std::out_of_range("out of range access");
    }
    size_t idx = (head_ + pos) % N;
    return data_.at(idx);
  }
  const T &at(std::size_t pos) const {
    if (pos >= size()) {
      throw std::out_of_range("out of range access");
    }
    size_t idx = (head_ + pos) % N;
    return data_.at(idx);
  }

  T &operator[](size_t pos) {
    assert(pos < size_);
    size_t idx = (head_ + pos) % N;
    return data_[idx];
  }

  const T &operator[](size_t pos) const {
    assert(pos < size_);
    size_t idx = (head_ + pos) % N;
    return data_[idx];
  }

  T &front() {
    assert(size() != 0);
    return data_[head_];
  }
  const T &front() const {
    assert(size() != 0);
    return data_[head_];
  }

  T &back() {
    assert(size() != 0);
    return data_[(tail_ + N - 1) % N];
  }
  const T &back() const {
    assert(size() != 0);
    return data_[(tail_ + N - 1) % N];
  }

  const bool empty() const { return size_ == 0; }
  const bool is_full() const { return size_ == N; }
  const std::size_t size() const { return size_; }
  const std::size_t max_size() const { return N; }

  bool push(const T &value) noexcept {
    if (is_full()) {
      return false;
    }

    data_[tail_] = value;
    tail_ = (tail_ + 1) % N;
    size_++;
    return true;
  }

  bool pop(T &res) noexcept {
    if (empty()) {
      return false;
    }
    T value = data_[head_];
    head_ = (head_ + 1) % N;
    size_--;

    res = value;

    return true;
  }

  bool peek(T &res) const noexcept {
    if (empty()) {
      return false;
    }
    res = data_[head_];
    return true;
  }

  size_t write(const T *data, size_t count) noexcept {
    assert(data != nullptr);
    size_t remainingSpace = N - size_;
    size_t amountToCopy = count <= remainingSpace ? count : remainingSpace;

    size_t i;
    for (i = 0; i < amountToCopy; i++) {
      push(data[i]);
    }

    return i;
  }

private:
  std::array<T, N> data_{};
  size_t head_ = 0;
  size_t tail_ = 0;
  size_t size_ = 0;
};
} // namespace audio
