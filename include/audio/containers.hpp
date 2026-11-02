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

  bool push(const T &value) {
    if (is_full()) {
      return false;
    }

    data_[tail_] = value;
    tail_ = (tail_ + 1) % N;
    size_++;
    return true;
  }

  bool pop(T &res) {
    if (empty()) {
      return false;
    }
    T value = data_[head_];
    head_ = (head_ + 1) % N;
    size_--;

    res = value;

    return true;
  }

  bool peek(T &res) const {
    if (empty()) {
      return false;
    }
    res = data_[head_];
    return true;
  }

  iterator begin() { return RingBufferIterator(this, 0); }
  const_iterator begin() const { return RingBufferIterator(this, 0); }
  const_iterator cbegin() const { return RingBufferIterator(this, 0); }

  iterator end() { return RingBufferIterator(this, size()); }
  const_iterator end() const { return RingBufferIterator(this, size()); }
  const_iterator cend() const { return RingBufferIterator(this, size()); }

  // TODO: Add reverse iterator support
  // TODO: Add fill/swap
  class RingBufferIterator {
  public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T &;

    RingBufferIterator(RingBuffer *ringBuffer)
        : ringBuffer_(ringBuffer), idx_(0) {}

    RingBufferIterator(RingBuffer *ringBuffer, size_t idx)
        : ringBuffer_(ringBuffer), idx_(idx) {}

    reference operator*() const {
      assert(ringBuffer_ != nullptr);

      return (*ringBuffer_)[idx_];
    }
    pointer operator->() const { return &(*(*this)); }

    reference operator++() {
      idx_++;
      return *this;
    };

    reference operator--() {
      idx_--;
      return *this;
    }

    RingBufferIterator operator++(int) {
      RingBufferIterator tmp = *this;
      ++(*this);
      return tmp;
    }

    RingBufferIterator operator--(int) {
      RingBufferIterator tmp = *this;
      --(*this);
      return tmp;
    }

    reference operator+=(difference_type n) {
      idx_ += n;
      return *this;
    }

    reference operator-=(difference_type n) { return *this += -n; }

    friend RingBufferIterator operator+(RingBufferIterator iterator,
                                        difference_type n) {
      iterator += n;
      return iterator;
    }

    friend RingBufferIterator operator+(difference_type n,
                                        RingBufferIterator iterator) {
      iterator += n;
      return iterator;
    }

    friend RingBufferIterator operator-(RingBufferIterator iterator,
                                        difference_type n) {
      iterator -= n;
      return iterator;
    }

    friend difference_type operator-(const RingBufferIterator &lhs,
                                     const RingBufferIterator &rhs) {
      return lhs.idx_ - rhs.idx_;
    }

    reference operator[](difference_type n) const { *(*this + n); }

    bool operator<(const RingBufferIterator &other) const {
      return other - *this > 0;
    }
    bool operator>(const RingBufferIterator &other) const {
      return other < *this;
    }
    bool operator>=(const RingBufferIterator &other) const {
      return !(*this < other);
    }
    bool operator<=(const RingBufferIterator &other) const {
      return !(*this > other);
    }

    bool operator==(const RingBufferIterator &other) const {
      return (ringBuffer_ == other.ringBuffer_ && idx_ == other.idx_);
    }

    bool operator!=(const RingBufferIterator &other) const {
      return !(*this == other);
    }

  private:
    RingBuffer *ringBuffer_;
    size_t idx_;
  };

private:
  std::array<T, N> data_{};
  size_t head_ = 0;
  size_t tail_ = 0;
  size_t size_ = 0;
};
} // namespace audio
