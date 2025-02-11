// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/containers/heap_array.h"

#include <stdint.h>

#include <type_traits>

#include "base/containers/span.h"
#include "base/memory/raw_ptr_exclusion.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace base {

namespace {

class DestructCounter {
 public:
  DestructCounter() = default;
  ~DestructCounter() {
    if (where_) {
      (*where_)++;
    }
  }

  void set_where(size_t* where) { where_ = where; }

 private:
  // RAW_PTR_EXCLUSION: Stack location only.
  RAW_PTR_EXCLUSION size_t* where_ = nullptr;
};

}  // namespace

TEST(HeapArray, DefaultConstructor) {
  HeapArray<uint32_t> vec;
  EXPECT_TRUE(vec.empty());
  EXPECT_EQ(vec.size(), 0u);
  EXPECT_EQ(vec.data(), nullptr);
}

TEST(HeapArray, WithSizeZero) {
  auto vec = HeapArray<uint32_t>::WithSize(0u);
  EXPECT_EQ(vec.size(), 0u);
  EXPECT_EQ(vec.data(), nullptr);
}

TEST(HeapArray, WithSizeNonZero) {
  auto vec = HeapArray<uint32_t>::WithSize(2u);
  EXPECT_EQ(vec.size(), 2u);
  EXPECT_NE(vec.data(), nullptr);
}

TEST(HeapArray, MoveConstructor) {
  auto that = HeapArray<uint32_t>::WithSize(2u);
  base::HeapArray<uint32_t> vec(std::move(that));
  EXPECT_EQ(vec.size(), 2u);
  EXPECT_NE(vec.data(), nullptr);
  EXPECT_EQ(that.size(), 0u);
  EXPECT_EQ(that.data(), nullptr);
}

TEST(HeapArray, MoveAssign) {
  auto that = HeapArray<uint32_t>::WithSize(2u);
  auto vec = HeapArray<uint32_t>::WithSize(4u);
  vec = std::move(that);
  EXPECT_EQ(vec.size(), 2u);
  EXPECT_NE(vec.data(), nullptr);
  EXPECT_EQ(that.size(), 0u);
  EXPECT_EQ(that.data(), nullptr);
}

TEST(HeapArray, DataAndIndex) {
  base::HeapArray<uint32_t> empty;
  EXPECT_EQ(nullptr, empty.data());

  auto vec = HeapArray<uint32_t>::WithSize(2u);
  vec[0] = 100u;
  vec[1] = 101u;
  EXPECT_EQ(vec.data()[0], 100u);
  EXPECT_EQ(vec.data()[1], 101u);
}

TEST(HeapArray, IteratorAndIndex) {
  const base::HeapArray<uint32_t> empty;
  static_assert(
      std::is_const_v<std::remove_reference_t<decltype(*empty.begin())>>);
  static_assert(
      std::is_const_v<std::remove_reference_t<decltype(*empty.end())>>);
  EXPECT_EQ(empty.begin(), empty.end());

  auto vec = HeapArray<uint32_t>::WithSize(2u);
  static_assert(
      !std::is_const_v<std::remove_reference_t<decltype(*vec.begin())>>);
  static_assert(
      !std::is_const_v<std::remove_reference_t<decltype(*vec.end())>>);
  vec[0] = 100u;
  vec[1] = 101u;
  uint32_t expected = 100;
  for (auto i : vec) {
    EXPECT_EQ(i, expected);
    ++expected;
  }
  EXPECT_EQ(expected, 102u);
}

TEST(HeapArrayDeathTest, BadIndex) {
  auto vec = HeapArray<uint32_t>::WithSize(2u);
  EXPECT_DEATH_IF_SUPPORTED(vec[2], "");
}

TEST(HeapArray, AsSpan) {
  {
    auto vec = HeapArray<uint32_t>::WithSize(2u);
    auto s = vec.as_span();
    static_assert(std::same_as<decltype(s), span<uint32_t>>);
    EXPECT_EQ(s.size(), 2u);
    EXPECT_EQ(s.data(), vec.data());
  }
  {
    const auto vec = HeapArray<uint32_t>::WithSize(2u);
    auto s = vec.as_span();
    static_assert(std::same_as<decltype(s), span<const uint32_t>>);
    EXPECT_EQ(s.size(), 2u);
    EXPECT_EQ(s.data(), vec.data());
  }
}

TEST(HeapArray, Subspan) {
  auto vec = HeapArray<uint32_t>::WithSize(4u);
  for (size_t i = 0; i < vec.size(); ++i) {
    vec[i] = i;
  }
  base::span<uint32_t> empty = vec.subspan(2, 0);
  EXPECT_TRUE(empty.empty());

  base::span<uint32_t> first = vec.subspan(0, 1);
  EXPECT_EQ(first.size(), 1u);
  EXPECT_EQ(first[0], 0u);

  base::span<uint32_t> mids = vec.subspan(1, 2);
  EXPECT_EQ(mids.size(), 2u);
  EXPECT_EQ(mids[0], 1u);
  EXPECT_EQ(mids[1], 2u);

  base::span<uint32_t> rest = vec.subspan(3);
  EXPECT_EQ(rest.size(), 1u);
  EXPECT_EQ(rest[0], 3u);
}

TEST(HeapArray, First) {
  auto vec = HeapArray<uint32_t>::WithSize(4u);
  for (size_t i = 0; i < vec.size(); ++i) {
    vec[i] = i;
  }
  base::span<uint32_t> empty = vec.first(0);
  EXPECT_TRUE(empty.empty());

  base::span<uint32_t> some = vec.first(2);
  EXPECT_EQ(some.size(), 2u);
  EXPECT_EQ(some[0], 0u);
  EXPECT_EQ(some[1], 1u);
}

TEST(HeapArray, Last) {
  auto vec = HeapArray<uint32_t>::WithSize(4u);
  for (size_t i = 0; i < vec.size(); ++i) {
    vec[i] = i;
  }
  base::span<uint32_t> empty = vec.first(0);
  EXPECT_TRUE(empty.empty());

  base::span<uint32_t> some = vec.first(2);
  EXPECT_EQ(some.size(), 2u);
  EXPECT_EQ(some[0], 0u);
  EXPECT_EQ(some[1], 1u);
}

TEST(HeapArray, Init) {
  auto vec = base::HeapArray<uint32_t>::WithSize(200);
  EXPECT_EQ(0u, vec[0]);
  EXPECT_EQ(0u, vec[199]);

  uint32_t accumulator = 0;
  for (auto i : vec) {
    accumulator |= i;
  }
  EXPECT_EQ(0u, accumulator);
}

TEST(HeapArray, Uninit) {
  auto vec = base::HeapArray<uint32_t>::Uninit(4);
  vec[0] = 100u;
  vec[1] = 101u;
  EXPECT_EQ(100u, vec[0]);
  EXPECT_EQ(101u, vec[1]);
#if defined(MEMORY_SANITIZER)
  // TODO(tsepez): figure out how to get a msan crash here.
  // volatile uint32_t* x = vec.data() + 2;
  // EXPECT_DEATH(*x, "");
#endif
}

TEST(HeapArray, CopiedFrom) {
  base::span<uint32_t> empty_span;
  auto empty_vec = base::HeapArray<uint32_t>::CopiedFrom(empty_span);
  EXPECT_EQ(0u, empty_vec.size());

  const uint32_t kData[] = {1000u, 1001u};
  auto vec = base::HeapArray<uint32_t>::CopiedFrom(kData);
  ASSERT_EQ(2u, vec.size());
  EXPECT_EQ(1000u, vec[0]);
  EXPECT_EQ(1001u, vec[1]);
}

TEST(HeapArray, RunsDestructor) {
  size_t count = 0;
  {
    auto vec = base::HeapArray<DestructCounter>::WithSize(2);
    vec[0].set_where(&count);
    vec[1].set_where(&count);
    EXPECT_EQ(count, 0u);
  }
  EXPECT_EQ(count, 2u);
}

TEST(HeapArray, CopyFrom) {
  HeapArray<uint32_t> empty;
  HeapArray<uint32_t> something = HeapArray<uint32_t>::Uninit(2);
  HeapArray<uint32_t> other = HeapArray<uint32_t>::Uninit(2);
  const uint32_t kStuff[] = {1000u, 1001u};

  empty.copy_from(base::span<uint32_t>());  // Should not check.
  something.copy_from(kStuff);
  EXPECT_EQ(1000u, something[0]);
  EXPECT_EQ(1001u, something[1]);

  other.copy_from(something);
  EXPECT_EQ(1000u, other[0]);
  EXPECT_EQ(1001u, other[1]);
}

}  // namespace base
