#include "audio/containers.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

template <typename T, std::size_t N>
void ValidateRingBufferState(const audio::RingBuffer<T, N> &buf, size_t size,
                             size_t capacity, bool isFull, bool isEmpty) {
  EXPECT_EQ(buf.size(), size) << "RingBuffer reported incorrect size";
  EXPECT_EQ(buf.max_size(), capacity)
      << "RingBuffer reported incorrect capacity";
  EXPECT_EQ(buf.is_full(), isFull)
      << "RingBuffer reported incorrect full state";
  EXPECT_EQ(buf.empty(), isEmpty) << "RingBuffer reported incorrect full state";
}

TEST(RingBuffer, DefaultConstructor) {
  // Test Default Constructor
  audio::RingBuffer<int, 5> buf;
  ValidateRingBufferState(buf, 0, 5, false, true);
}

TEST(RingBuffer, InputIteratorConstructor) {
  // Test input iterator constructor
  std::vector<int> data({1, 2, 3, 4});

  audio::RingBuffer<int, 5> buf(data.begin(), data.end());
  ValidateRingBufferState(buf, 4, 5, false, false);

  EXPECT_EQ(buf.front(), 1)
      << "iterator initialized buffer reported incorrect front";
  EXPECT_EQ(buf.back(), 4)
      << "iterator initialized buffer reported incorrect front";
}

TEST(RingBuffer, InitializerListConstructor) {
  // Test with initializer list
  audio::RingBuffer<int, 5> buf({1, 2, 3, 4, 5, 6});
  ValidateRingBufferState(buf, 5, 5, true, false);

  EXPECT_EQ(buf.front(), 1)
      << "iterator initialized buffer reported incorrect front";
  EXPECT_EQ(buf.back(), 5)
      << "iterator initialized buffer reported incorrect front";
}

TEST(RingBuffer, RingBufferAccessOperators) {
  audio::RingBuffer<int, 5> emptyBuffer;

  EXPECT_THROW(
      { emptyBuffer.at(0); }, std::out_of_range)
      << "RingBuffer did not throw out of range";
  EXPECT_THROW(
      { emptyBuffer.at(-1); }, std::out_of_range)
      << "RingBuffer did not throw out of range";

  audio::RingBuffer<int, 5> nonEmptyBuffer({1, 2, 3});

  EXPECT_EQ(nonEmptyBuffer[0], 1);
  EXPECT_EQ(nonEmptyBuffer[1], 2);
  EXPECT_EQ(nonEmptyBuffer[2], 3);
  EXPECT_EQ(nonEmptyBuffer.at(0), 1);
  EXPECT_EQ(nonEmptyBuffer.at(1), 2);
  EXPECT_EQ(nonEmptyBuffer.at(2), 3);
  EXPECT_THROW({ nonEmptyBuffer.at(4); }, std::out_of_range);
  EXPECT_THROW({ nonEmptyBuffer.at(5); }, std::out_of_range);

  // test const variants
  /*  const audio::RingBuffer<int, 5> constBuf = nonEmptyBuffer;
    EXPECT_THROW(
        { constBuf.at(-1); }, std::out_of_range)
        << "RingBuffer did not through out of range";
    EXPECT_EQ(constBuf[0], 1);
    EXPECT_EQ(constBuf[1], 2);
    EXPECT_EQ(constBuf[2], 3);
    EXPECT_EQ(constBuf.at(0), 1);
    EXPECT_EQ(constBuf.at(1), 2);
    EXPECT_EQ(constBuf.at(2), 3);
    EXPECT_THROW({ constBuf.at(4); }, std::out_of_range);
    EXPECT_THROW({ constBuf.at(5); }, std::out_of_range);

    // front/back for both const and non-const variants
    EXPECT_EQ(nonEmptyBuffer.front(), 1);
    EXPECT_EQ(nonEmptyBuffer.back(), 3);
    EXPECT_EQ(constBuf.front(), 1);
    EXPECT_EQ(constBuf.back(), 3);*/
}

TEST(RingBuffer, RingBufferPopPushPeek) {
  audio::RingBuffer<int, 3> buf;
  int a, b;

  // ensure runtime errors occur when accessing empty buffer
  EXPECT_EQ(buf.pop(a), false);
  EXPECT_EQ(buf.peek(a), false);

  EXPECT_EQ(buf.push(3), true);
  ValidateRingBufferState(buf, 1, 3, false, false);
  EXPECT_EQ(buf.front(), 3);
  EXPECT_EQ(buf.back(), 3);
  EXPECT_EQ(buf.peek(b), true);
  EXPECT_EQ(b, 3);

  EXPECT_EQ(buf.pop(a), true);
  EXPECT_EQ(a, 3);
  ValidateRingBufferState(buf, 0, 3, false, true);

  // fill buffer
  EXPECT_EQ(buf.push(10), true);
  EXPECT_EQ(buf.push(20), true);
  EXPECT_EQ(buf.push(30), true);
  ValidateRingBufferState(buf, 3, 3, true, false);

  EXPECT_EQ(buf.peek(b), true);
  EXPECT_EQ(b, 10);

  // write to full buffer should not overwrite
  EXPECT_EQ(buf.push(40), false);
  ValidateRingBufferState(buf, 3, 3, true, false);
  EXPECT_EQ(buf.peek(b), true);
  EXPECT_EQ(b, 10);

  EXPECT_EQ(buf.pop(a), true);
  EXPECT_EQ(a, 10);
  ValidateRingBufferState(buf, 2, 3, false, false);

  // empty buffer
  EXPECT_EQ(buf.pop(a), true);
  EXPECT_EQ(buf.pop(a), true);

  ValidateRingBufferState(buf, 0, 3, false, true);
  EXPECT_EQ(buf.pop(a), false);
  EXPECT_EQ(buf.peek(a), false);

  // ensure write after emptying still functions properly
  buf.push(100);
  ValidateRingBufferState(buf, 1, 3, false, false);
  EXPECT_EQ(buf.front(), 100);
  EXPECT_EQ(buf.back(), 100);
  EXPECT_EQ(buf.peek(b), true);
  EXPECT_EQ(b, 100);
}

TEST(RingBuffer, WrapAround) {
  audio::RingBuffer<int, 10> buf;

  // write and consume in intervals of 5 to ensure that the buffer handles
  // wrapping properly
  int res;
  for (int i = 0; i < 6; i++) {
    // push 5
    for (int j = 0; j < 5; j++) {
      buf.push(i * 5 + j);
    }
    // read 4
    for (int j = 0; j < 4; j++) {
      buf.pop(res);
    }

    buf.peek(res);
    EXPECT_EQ(res, (i + 1) * 4);
    EXPECT_EQ(buf.size(), i + 1);
  }

  for (int i = 0; i < 6; i++) {
    ASSERT_EQ(buf.pop(res), true);
    ASSERT_EQ(res, 24 + i);
  }

  ValidateRingBufferState(buf, 0, 10, false, true);
}

TEST(RingBuffer, BulkWrite) {
  int arr[] = {1, 2, 3, 4, 5};
  audio::RingBuffer<int, 5> buf;

  EXPECT_EQ(buf.write(arr, 0), 0);
  ValidateRingBufferState(buf, 0, 5, false, true);

  // partial write
  EXPECT_EQ(buf.write(arr, 2), 2);
  ValidateRingBufferState(buf, 2, 5, false, false);
  EXPECT_EQ(buf.front(), 1);
  EXPECT_EQ(buf.back(), 2);

  // overwrite
  EXPECT_EQ(buf.write(arr, 5), 3);
  EXPECT_EQ(buf.front(), 1);
  EXPECT_EQ(buf.back(), 3);
  ValidateRingBufferState(buf, 5, 5, true, false);
  EXPECT_EQ(buf.write(arr, 5), 0);
  ValidateRingBufferState(buf, 5, 5, true, false);
}

// TODO: write iterator tests
