// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/containers/span.h"

#include <stdint.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "base/containers/adapters.h"
#include "base/containers/checked_iterators.h"
#include "base/ranges/algorithm.h"
#include "base/strings/string_piece.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::Pointwise;

namespace base {

namespace {

// Tests for span(It, StrictNumeric<size_t>) deduction guide. These tests use a
// helper function to wrap the static_asserts, as most STL containers don't work
// well in a constexpr context. std::array<T, N> does, but base::span has
// specific overloads for std::array<T, n>, so that ends up being less helpful
// than it would initially appear.
//
// Another alternative would be to use std::declval, but that would be fairly
// verbose.
[[maybe_unused]] void TestDeductionGuides() {
  // Tests for span(It, EndOrSize) deduction guide.
  {
    const std::vector<int> v;
    static_assert(
        std::is_same_v<decltype(span(v.cbegin(), v.size())), span<const int>>);
    static_assert(
        std::is_same_v<decltype(span(v.begin(), v.size())), span<const int>>);
    static_assert(
        std::is_same_v<decltype(span(v.data(), v.size())), span<const int>>);
  }

  {
    std::vector<int> v;
    static_assert(
        std::is_same_v<decltype(span(v.cbegin(), v.size())), span<const int>>);
    static_assert(
        std::is_same_v<decltype(span(v.begin(), v.size())), span<int>>);
    static_assert(
        std::is_same_v<decltype(span(v.data(), v.size())), span<int>>);
  }

  {
    const std::vector<int> v;
    static_assert(
        std::is_same_v<decltype(span(v.cbegin(), v.cend())), span<const int>>);
    static_assert(
        std::is_same_v<decltype(span(v.begin(), v.end())), span<const int>>);
  }

  {
    std::vector<int> v;
    static_assert(
        std::is_same_v<decltype(span(v.cbegin(), v.cend())), span<const int>>);
    static_assert(
        std::is_same_v<decltype(span(v.begin(), v.end())), span<int>>);
  }

  // Tests for span(Range&&) deduction guide.
  {
    const int kArray[] = {1, 2, 3};
    static_assert(std::is_same_v<decltype(span(kArray)), span<const int, 3>>);
  }
  {
    int kArray[] = {1, 2, 3};
    static_assert(std::is_same_v<decltype(span(kArray)), span<int, 3>>);
  }

  static_assert(
      std::is_same_v<decltype(span(std::declval<std::array<const bool, 3>&>())),
                     span<const bool, 3>>);
  static_assert(
      std::is_same_v<decltype(span(std::declval<std::array<bool, 3>&>())),
                     span<bool, 3>>);

  static_assert(
      std::is_same_v<decltype(span(
                         std::declval<const std::array<const bool, 3>&>())),
                     span<const bool, 3>>);
  static_assert(
      std::is_same_v<decltype(span(
                         std::declval<const std::array<const bool, 3>&&>())),
                     span<const bool, 3>>);
  static_assert(std::is_same_v<
                decltype(span(std::declval<std::array<const bool, 3>&&>())),
                span<const bool, 3>>);
  static_assert(
      std::is_same_v<decltype(span(std::declval<const std::array<bool, 3>&>())),
                     span<const bool, 3>>);
  static_assert(std::is_same_v<
                decltype(span(std::declval<const std::array<bool, 3>&&>())),
                span<const bool, 3>>);
  static_assert(
      std::is_same_v<decltype(span(std::declval<std::array<bool, 3>&&>())),
                     span<const bool, 3>>);

  static_assert(
      std::is_same_v<decltype(span(std::declval<const std::string&>())),
                     span<const char>>);
  static_assert(
      std::is_same_v<decltype(span(std::declval<const std::string&&>())),
                     span<const char>>);
  static_assert(
      std::is_same_v<decltype(span(std::declval<std::string&>())), span<char>>);
  static_assert(std::is_same_v<decltype(span(std::declval<std::string&&>())),
                               span<const char>>);
  static_assert(
      std::is_same_v<decltype(span(std::declval<const std::u16string&>())),
                     span<const char16_t>>);
  static_assert(
      std::is_same_v<decltype(span(std::declval<const std::u16string&&>())),
                     span<const char16_t>>);
  static_assert(std::is_same_v<decltype(span(std::declval<std::u16string&>())),
                               span<char16_t>>);
  static_assert(std::is_same_v<decltype(span(std::declval<std::u16string&&>())),
                               span<const char16_t>>);
  static_assert(std::is_same_v<
                decltype(span(std::declval<const std::array<float, 9>&>())),
                span<const float, 9>>);
  static_assert(std::is_same_v<
                decltype(span(std::declval<const std::array<float, 9>&&>())),
                span<const float, 9>>);
  static_assert(
      std::is_same_v<decltype(span(std::declval<std::array<float, 9>&>())),
                     span<float, 9>>);
  static_assert(
      std::is_same_v<decltype(span(std::declval<std::array<float, 9>&&>())),
                     span<const float, 9>>);
}

}  // namespace

TEST(SpanTest, DefaultConstructor) {
  span<int> dynamic_span;
  EXPECT_EQ(nullptr, dynamic_span.data());
  EXPECT_EQ(0u, dynamic_span.size());

  constexpr span<int, 0> static_span;
  static_assert(nullptr == static_span.data(), "");
  static_assert(0u == static_span.size(), "");
}

TEST(SpanTest, ConstructFromDataAndSize) {
  constexpr int* kNull = nullptr;
  constexpr span<int> empty_span(kNull, 0u);
  EXPECT_TRUE(empty_span.empty());
  EXPECT_EQ(nullptr, empty_span.data());

  std::vector<int> vector = {1, 1, 2, 3, 5, 8};

  span<int> dynamic_span(vector.data(), vector.size());
  EXPECT_EQ(vector.data(), dynamic_span.data());
  EXPECT_EQ(vector.size(), dynamic_span.size());

  for (size_t i = 0; i < dynamic_span.size(); ++i)
    EXPECT_EQ(vector[i], dynamic_span[i]);

  span<int, 6> static_span(vector.data(), vector.size());
  EXPECT_EQ(vector.data(), static_span.data());
  EXPECT_EQ(vector.size(), static_span.size());

  for (size_t i = 0; i < static_span.size(); ++i)
    EXPECT_EQ(vector[i], static_span[i]);
}

TEST(SpanTest, ConstructFromIterAndSize) {
  constexpr int* kNull = nullptr;
  constexpr span<int> empty_span(kNull, 0u);
  EXPECT_TRUE(empty_span.empty());
  EXPECT_EQ(nullptr, empty_span.data());

  std::vector<int> vector = {1, 1, 2, 3, 5, 8};

  span<int> dynamic_span(vector.begin(), vector.size());
  EXPECT_EQ(vector.data(), dynamic_span.data());
  EXPECT_EQ(vector.size(), dynamic_span.size());

  for (size_t i = 0; i < dynamic_span.size(); ++i)
    EXPECT_EQ(vector[i], dynamic_span[i]);

  span<int, 6> static_span(vector.begin(), vector.size());
  EXPECT_EQ(vector.data(), static_span.data());
  EXPECT_EQ(vector.size(), static_span.size());

  for (size_t i = 0; i < static_span.size(); ++i)
    EXPECT_EQ(vector[i], static_span[i]);
}

TEST(SpanTest, ConstructFromIterPair) {
  constexpr int* kNull = nullptr;
  constexpr span<int> empty_span(kNull, kNull);
  EXPECT_TRUE(empty_span.empty());
  EXPECT_EQ(nullptr, empty_span.data());

  std::vector<int> vector = {1, 1, 2, 3, 5, 8};

  span<int> dynamic_span(vector.begin(), vector.begin() + vector.size() / 2);
  EXPECT_EQ(vector.data(), dynamic_span.data());
  EXPECT_EQ(vector.size() / 2, dynamic_span.size());

  for (size_t i = 0; i < dynamic_span.size(); ++i)
    EXPECT_EQ(vector[i], dynamic_span[i]);

  span<int, 3> static_span(vector.begin(), vector.begin() + vector.size() / 2);
  EXPECT_EQ(vector.data(), static_span.data());
  EXPECT_EQ(vector.size() / 2, static_span.size());

  for (size_t i = 0; i < static_span.size(); ++i)
    EXPECT_EQ(vector[i], static_span[i]);
}

TEST(SpanTest, AllowedConversionsFromStdArray) {
  // In the following assertions we use std::is_convertible_v<From, To>, which
  // for non-void types is equivalent to checking whether the following
  // expression is well-formed:
  //
  // T obj = std::declval<From>();
  //
  // In particular we are checking whether From is implicitly convertible to To,
  // which also implies that To is explicitly constructible from From.
  static_assert(
      std::is_convertible_v<std::array<int, 3>&, base::span<int>>,
      "Error: l-value reference to std::array<int> should be convertible to "
      "base::span<int> with dynamic extent.");
  static_assert(
      std::is_convertible_v<std::array<int, 3>&, base::span<int, 3>>,
      "Error: l-value reference to std::array<int> should be convertible to "
      "base::span<int> with the same static extent.");
  static_assert(
      std::is_convertible_v<std::array<int, 3>&, base::span<const int>>,
      "Error: l-value reference to std::array<int> should be convertible to "
      "base::span<const int> with dynamic extent.");
  static_assert(
      std::is_convertible_v<std::array<int, 3>&, base::span<const int, 3>>,
      "Error: l-value reference to std::array<int> should be convertible to "
      "base::span<const int> with the same static extent.");
  static_assert(
      std::is_convertible_v<const std::array<int, 3>&, base::span<const int>>,
      "Error: const l-value reference to std::array<int> should be "
      "convertible to base::span<const int> with dynamic extent.");
  static_assert(
      std::is_convertible_v<const std::array<int, 3>&,
                            base::span<const int, 3>>,
      "Error: const l-value reference to std::array<int> should be convertible "
      "to base::span<const int> with the same static extent.");
  static_assert(
      std::is_convertible_v<std::array<const int, 3>&, base::span<const int>>,
      "Error: l-value reference to std::array<const int> should be "
      "convertible to base::span<const int> with dynamic extent.");
  static_assert(
      std::is_convertible_v<std::array<const int, 3>&,
                            base::span<const int, 3>>,
      "Error: l-value reference to std::array<const int> should be convertible "
      "to base::span<const int> with the same static extent.");
  static_assert(
      std::is_convertible_v<const std::array<const int, 3>&,
                            base::span<const int>>,
      "Error: const l-value reference to std::array<const int> should be "
      "convertible to base::span<const int> with dynamic extent.");
  static_assert(
      std::is_convertible_v<const std::array<const int, 3>&,
                            base::span<const int, 3>>,
      "Error: const l-value reference to std::array<const int> should be "
      "convertible to base::span<const int> with the same static extent.");
}

TEST(SpanTest, DisallowedConstructionsFromStdArray) {
  // In the following assertions we use !std::is_constructible_v<T, Args>, which
  // is equivalent to checking whether the following expression is malformed:
  //
  // T obj(std::declval<Args>()...);
  //
  // In particular we are checking that T is not explicitly constructible from
  // Args, which also implies that T is not implicitly constructible from Args
  // as well.
  static_assert(
      !std::is_constructible_v<base::span<int>, const std::array<int, 3>&>,
      "Error: base::span<int> with dynamic extent should not be constructible "
      "from const l-value reference to std::array<int>");

  static_assert(
      !std::is_constructible_v<base::span<int>, std::array<const int, 3>&>,
      "Error: base::span<int> with dynamic extent should not be constructible "
      "from l-value reference to std::array<const int>");

  static_assert(
      !std::is_constructible_v<base::span<int>,
                               const std::array<const int, 3>&>,
      "Error: base::span<int> with dynamic extent should not be constructible "
      "const from l-value reference to std::array<const int>");

  static_assert(
      !std::is_constructible_v<base::span<int, 2>, std::array<int, 3>&>,
      "Error: base::span<int> with static extent should not be constructible "
      "from l-value reference to std::array<int> with different extent");

  static_assert(
      !std::is_constructible_v<base::span<int, 4>, std::array<int, 3>&>,
      "Error: base::span<int> with dynamic extent should not be constructible "
      "from l-value reference to std::array<int> with different extent");

  static_assert(
      !std::is_constructible_v<base::span<int>, std::array<bool, 3>&>,
      "Error: base::span<int> with dynamic extent should not be constructible "
      "from l-value reference to std::array<bool>");
}

TEST(SpanTest, ConstructFromConstexprArray) {
  static constexpr int kArray[] = {5, 4, 3, 2, 1};

  constexpr span<const int> dynamic_span(kArray);
  static_assert(kArray == dynamic_span.data(), "");
  static_assert(std::size(kArray) == dynamic_span.size(), "");

  static_assert(kArray[0] == dynamic_span[0], "");
  static_assert(kArray[1] == dynamic_span[1], "");
  static_assert(kArray[2] == dynamic_span[2], "");
  static_assert(kArray[3] == dynamic_span[3], "");
  static_assert(kArray[4] == dynamic_span[4], "");

  constexpr span<const int, std::size(kArray)> static_span(kArray);
  static_assert(kArray == static_span.data(), "");
  static_assert(std::size(kArray) == static_span.size(), "");

  static_assert(kArray[0] == static_span[0], "");
  static_assert(kArray[1] == static_span[1], "");
  static_assert(kArray[2] == static_span[2], "");
  static_assert(kArray[3] == static_span[3], "");
  static_assert(kArray[4] == static_span[4], "");
}

TEST(SpanTest, ConstructFromArray) {
  int array[] = {5, 4, 3, 2, 1};

  span<const int> const_span(array);
  EXPECT_EQ(array, const_span.data());
  EXPECT_EQ(std::size(array), const_span.size());
  for (size_t i = 0; i < const_span.size(); ++i)
    EXPECT_EQ(array[i], const_span[i]);

  span<int> dynamic_span(array);
  EXPECT_EQ(array, dynamic_span.data());
  EXPECT_EQ(std::size(array), dynamic_span.size());
  for (size_t i = 0; i < dynamic_span.size(); ++i)
    EXPECT_EQ(array[i], dynamic_span[i]);

  span<int, std::size(array)> static_span(array);
  EXPECT_EQ(array, static_span.data());
  EXPECT_EQ(std::size(array), static_span.size());
  for (size_t i = 0; i < static_span.size(); ++i)
    EXPECT_EQ(array[i], static_span[i]);
}

TEST(SpanTest, ConstructFromVolatileArray) {
  static volatile int array[] = {5, 4, 3, 2, 1};

  span<const volatile int> const_span(array);
  static_assert(std::is_same_v<decltype(&const_span[1]), const volatile int*>);
  static_assert(
      std::is_same_v<decltype(const_span.data()), const volatile int*>);
  EXPECT_EQ(array, const_span.data());
  EXPECT_EQ(std::size(array), const_span.size());
  for (size_t i = 0; i < const_span.size(); ++i) {
    EXPECT_EQ(array[i], const_span[i]);
  }

  span<volatile int> dynamic_span(array);
  static_assert(std::is_same_v<decltype(&dynamic_span[1]), volatile int*>);
  static_assert(std::is_same_v<decltype(dynamic_span.data()), volatile int*>);
  EXPECT_EQ(array, dynamic_span.data());
  EXPECT_EQ(std::size(array), dynamic_span.size());
  for (size_t i = 0; i < dynamic_span.size(); ++i) {
    EXPECT_EQ(array[i], dynamic_span[i]);
  }

  span<volatile int, std::size(array)> static_span(array);
  static_assert(std::is_same_v<decltype(&static_span[1]), volatile int*>);
  static_assert(std::is_same_v<decltype(static_span.data()), volatile int*>);
  EXPECT_EQ(array, static_span.data());
  EXPECT_EQ(std::size(array), static_span.size());
  for (size_t i = 0; i < static_span.size(); ++i) {
    EXPECT_EQ(array[i], static_span[i]);
  }
}

TEST(SpanTest, ConstructFromStdArray) {
  // Note: Constructing a constexpr span from a constexpr std::array does not
  // work prior to C++17 due to non-constexpr std::array::data.
  std::array<int, 5> array = {{5, 4, 3, 2, 1}};

  span<const int> const_span(array);
  EXPECT_EQ(array.data(), const_span.data());
  EXPECT_EQ(array.size(), const_span.size());
  for (size_t i = 0; i < const_span.size(); ++i)
    EXPECT_EQ(array[i], const_span[i]);

  span<int> dynamic_span(array);
  EXPECT_EQ(array.data(), dynamic_span.data());
  EXPECT_EQ(array.size(), dynamic_span.size());
  for (size_t i = 0; i < dynamic_span.size(); ++i)
    EXPECT_EQ(array[i], dynamic_span[i]);

  span<int, std::size(array)> static_span(array);
  EXPECT_EQ(array.data(), static_span.data());
  EXPECT_EQ(array.size(), static_span.size());
  for (size_t i = 0; i < static_span.size(); ++i)
    EXPECT_EQ(array[i], static_span[i]);
}

TEST(SpanTest, ConstructFromInitializerList) {
  std::initializer_list<int> il = {1, 1, 2, 3, 5, 8};

  span<const int> const_span(il);
  EXPECT_EQ(il.begin(), const_span.data());
  EXPECT_EQ(il.size(), const_span.size());

  for (size_t i = 0; i < const_span.size(); ++i)
    EXPECT_EQ(il.begin()[i], const_span[i]);

  span<const int, 6> static_span(il.begin(), il.end());
  EXPECT_EQ(il.begin(), static_span.data());
  EXPECT_EQ(il.size(), static_span.size());

  for (size_t i = 0; i < static_span.size(); ++i)
    EXPECT_EQ(il.begin()[i], static_span[i]);
}

TEST(SpanTest, ConstructFromStdString) {
  std::string str = "foobar";

  span<const char> const_span(str);
  EXPECT_EQ(str.data(), const_span.data());
  EXPECT_EQ(str.size(), const_span.size());

  for (size_t i = 0; i < const_span.size(); ++i)
    EXPECT_EQ(str[i], const_span[i]);

  span<char> dynamic_span(str);
  EXPECT_EQ(str.data(), dynamic_span.data());
  EXPECT_EQ(str.size(), dynamic_span.size());

  for (size_t i = 0; i < dynamic_span.size(); ++i)
    EXPECT_EQ(str[i], dynamic_span[i]);

  span<char, 6> static_span(data(str), str.size());
  EXPECT_EQ(str.data(), static_span.data());
  EXPECT_EQ(str.size(), static_span.size());

  for (size_t i = 0; i < static_span.size(); ++i)
    EXPECT_EQ(str[i], static_span[i]);
}

TEST(SpanTest, ConstructFromConstContainer) {
  const std::vector<int> vector = {1, 1, 2, 3, 5, 8};

  span<const int> const_span(vector);
  EXPECT_EQ(vector.data(), const_span.data());
  EXPECT_EQ(vector.size(), const_span.size());

  for (size_t i = 0; i < const_span.size(); ++i)
    EXPECT_EQ(vector[i], const_span[i]);

  span<const int, 6> static_span(vector.data(), vector.size());
  EXPECT_EQ(vector.data(), static_span.data());
  EXPECT_EQ(vector.size(), static_span.size());

  for (size_t i = 0; i < static_span.size(); ++i)
    EXPECT_EQ(vector[i], static_span[i]);
}

TEST(SpanTest, ConstructFromContainer) {
  std::vector<int> vector = {1, 1, 2, 3, 5, 8};

  span<const int> const_span(vector);
  EXPECT_EQ(vector.data(), const_span.data());
  EXPECT_EQ(vector.size(), const_span.size());

  for (size_t i = 0; i < const_span.size(); ++i)
    EXPECT_EQ(vector[i], const_span[i]);

  span<int> dynamic_span(vector);
  EXPECT_EQ(vector.data(), dynamic_span.data());
  EXPECT_EQ(vector.size(), dynamic_span.size());

  for (size_t i = 0; i < dynamic_span.size(); ++i)
    EXPECT_EQ(vector[i], dynamic_span[i]);

  span<int, 6> static_span(vector.data(), vector.size());
  EXPECT_EQ(vector.data(), static_span.data());
  EXPECT_EQ(vector.size(), static_span.size());

  for (size_t i = 0; i < static_span.size(); ++i)
    EXPECT_EQ(vector[i], static_span[i]);
}

TEST(SpanTest, FromRefOfMutableStackVariable) {
  int x = 123;

  auto s = span_from_ref(x);
  static_assert(std::is_same_v<decltype(s), span<int, 1u>>);
  EXPECT_EQ(&x, s.data());
  EXPECT_EQ(1u, s.size());
  EXPECT_EQ(sizeof(int), s.size_bytes());
  EXPECT_EQ(123, s[0]);

  s[0] = 456;
  EXPECT_EQ(456, x);
  EXPECT_EQ(456, s[0]);
}

TEST(SpanTest, FromRefOfConstStackVariable) {
  const int x = 123;

  auto s = span_from_ref(x);
  static_assert(std::is_same_v<decltype(s), span<const int, 1u>>);
  EXPECT_EQ(&x, s.data());
  EXPECT_EQ(1u, s.size());
  EXPECT_EQ(sizeof(int), s.size_bytes());
  EXPECT_EQ(123, s[0]);
}

TEST(SpanTest, ConvertNonConstIntegralToConst) {
  std::vector<int> vector = {1, 1, 2, 3, 5, 8};

  span<int> int_span(vector.data(), vector.size());
  span<const int> const_span(int_span);
  EXPECT_EQ(int_span.size(), const_span.size());

  EXPECT_THAT(const_span, Pointwise(Eq(), int_span));

  span<int, 6> static_int_span(vector.data(), vector.size());
  span<const int, 6> static_const_span(static_int_span);
  EXPECT_THAT(static_const_span, Pointwise(Eq(), static_int_span));
}

TEST(SpanTest, ConvertNonConstPointerToConst) {
  auto a = std::make_unique<int>(11);
  auto b = std::make_unique<int>(22);
  auto c = std::make_unique<int>(33);
  std::vector<int*> vector = {a.get(), b.get(), c.get()};

  span<int*> non_const_pointer_span(vector);
  EXPECT_THAT(non_const_pointer_span, Pointwise(Eq(), vector));
  span<int* const> const_pointer_span(non_const_pointer_span);
  EXPECT_THAT(const_pointer_span, Pointwise(Eq(), non_const_pointer_span));
  // Note: no test for conversion from span<int> to span<const int*>, since that
  // would imply a conversion from int** to const int**, which is unsafe.
  //
  // Note: no test for conversion from span<int*> to span<const int* const>,
  // due to CWG Defect 330:
  // http://open-std.org/JTC1/SC22/WG21/docs/cwg_defects.html#330

  span<int*, 3> static_non_const_pointer_span(vector.data(), vector.size());
  EXPECT_THAT(static_non_const_pointer_span, Pointwise(Eq(), vector));
  span<int* const, 3> static_const_pointer_span(static_non_const_pointer_span);
  EXPECT_THAT(static_const_pointer_span,
              Pointwise(Eq(), static_non_const_pointer_span));
}

TEST(SpanTest, ConvertBetweenEquivalentTypes) {
  std::vector<int32_t> vector = {2, 4, 8, 16, 32};

  span<int32_t> int32_t_span(vector);
  span<int> converted_span(int32_t_span);
  EXPECT_EQ(int32_t_span.data(), converted_span.data());
  EXPECT_EQ(int32_t_span.size(), converted_span.size());

  span<int32_t, 5> static_int32_t_span(vector.data(), vector.size());
  span<int, 5> static_converted_span(static_int32_t_span);
  EXPECT_EQ(static_int32_t_span.data(), static_converted_span.data());
  EXPECT_EQ(static_int32_t_span.size(), static_converted_span.size());
}

TEST(SpanTest, TemplatedFirst) {
  static constexpr int array[] = {1, 2, 3};
  constexpr span<const int, 3> span(array);

  {
    constexpr auto subspan = span.first<0>();
    static_assert(span.data() == subspan.data(), "");
    static_assert(0u == subspan.size(), "");
    static_assert(0u == decltype(subspan)::extent, "");
  }

  {
    constexpr auto subspan = span.first<1>();
    static_assert(span.data() == subspan.data(), "");
    static_assert(1u == subspan.size(), "");
    static_assert(1u == decltype(subspan)::extent, "");
    static_assert(1 == subspan[0], "");
  }

  {
    constexpr auto subspan = span.first<2>();
    static_assert(span.data() == subspan.data(), "");
    static_assert(2u == subspan.size(), "");
    static_assert(2u == decltype(subspan)::extent, "");
    static_assert(1 == subspan[0], "");
    static_assert(2 == subspan[1], "");
  }

  {
    constexpr auto subspan = span.first<3>();
    static_assert(span.data() == subspan.data(), "");
    static_assert(3u == subspan.size(), "");
    static_assert(3u == decltype(subspan)::extent, "");
    static_assert(1 == subspan[0], "");
    static_assert(2 == subspan[1], "");
    static_assert(3 == subspan[2], "");
  }
}

TEST(SpanTest, TemplatedLast) {
  static constexpr int array[] = {1, 2, 3};
  constexpr span<const int, 3> span(array);

  {
    constexpr auto subspan = span.last<0>();
    static_assert(span.data() + 3 == subspan.data(), "");
    static_assert(0u == subspan.size(), "");
    static_assert(0u == decltype(subspan)::extent, "");
  }

  {
    constexpr auto subspan = span.last<1>();
    static_assert(span.data() + 2 == subspan.data(), "");
    static_assert(1u == subspan.size(), "");
    static_assert(1u == decltype(subspan)::extent, "");
    static_assert(3 == subspan[0], "");
  }

  {
    constexpr auto subspan = span.last<2>();
    static_assert(span.data() + 1 == subspan.data(), "");
    static_assert(2u == subspan.size(), "");
    static_assert(2u == decltype(subspan)::extent, "");
    static_assert(2 == subspan[0], "");
    static_assert(3 == subspan[1], "");
  }

  {
    constexpr auto subspan = span.last<3>();
    static_assert(span.data() == subspan.data(), "");
    static_assert(3u == subspan.size(), "");
    static_assert(3u == decltype(subspan)::extent, "");
    static_assert(1 == subspan[0], "");
    static_assert(2 == subspan[1], "");
    static_assert(3 == subspan[2], "");
  }
}

TEST(SpanTest, TemplatedSubspan) {
  static constexpr int array[] = {1, 2, 3};
  constexpr span<const int, 3> span(array);

  {
    constexpr auto subspan = span.subspan<0>();
    static_assert(span.data() == subspan.data(), "");
    static_assert(3u == subspan.size(), "");
    static_assert(3u == decltype(subspan)::extent, "");
    static_assert(1 == subspan[0], "");
    static_assert(2 == subspan[1], "");
    static_assert(3 == subspan[2], "");
  }

  {
    constexpr auto subspan = span.subspan<1>();
    static_assert(span.data() + 1 == subspan.data(), "");
    static_assert(2u == subspan.size(), "");
    static_assert(2u == decltype(subspan)::extent, "");
    static_assert(2 == subspan[0], "");
    static_assert(3 == subspan[1], "");
  }

  {
    constexpr auto subspan = span.subspan<2>();
    static_assert(span.data() + 2 == subspan.data(), "");
    static_assert(1u == subspan.size(), "");
    static_assert(1u == decltype(subspan)::extent, "");
    static_assert(3 == subspan[0], "");
  }

  {
    constexpr auto subspan = span.subspan<3>();
    static_assert(span.data() + 3 == subspan.data(), "");
    static_assert(0u == subspan.size(), "");
    static_assert(0u == decltype(subspan)::extent, "");
  }

  {
    constexpr auto subspan = span.subspan<0, 0>();
    static_assert(span.data() == subspan.data(), "");
    static_assert(0u == subspan.size(), "");
    static_assert(0u == decltype(subspan)::extent, "");
  }

  {
    constexpr auto subspan = span.subspan<1, 0>();
    static_assert(span.data() + 1 == subspan.data(), "");
    static_assert(0u == subspan.size(), "");
    static_assert(0u == decltype(subspan)::extent, "");
  }

  {
    constexpr auto subspan = span.subspan<2, 0>();
    static_assert(span.data() + 2 == subspan.data(), "");
    static_assert(0u == subspan.size(), "");
    static_assert(0u == decltype(subspan)::extent, "");
  }

  {
    constexpr auto subspan = span.subspan<0, 1>();
    static_assert(span.data() == subspan.data(), "");
    static_assert(1u == subspan.size(), "");
    static_assert(1u == decltype(subspan)::extent, "");
    static_assert(1 == subspan[0], "");
  }

  {
    constexpr auto subspan = span.subspan<1, 1>();
    static_assert(span.data() + 1 == subspan.data(), "");
    static_assert(1u == subspan.size(), "");
    static_assert(1u == decltype(subspan)::extent, "");
    static_assert(2 == subspan[0], "");
  }

  {
    constexpr auto subspan = span.subspan<2, 1>();
    static_assert(span.data() + 2 == subspan.data(), "");
    static_assert(1u == subspan.size(), "");
    static_assert(1u == decltype(subspan)::extent, "");
    static_assert(3 == subspan[0], "");
  }

  {
    constexpr auto subspan = span.subspan<0, 2>();
    static_assert(span.data() == subspan.data(), "");
    static_assert(2u == subspan.size(), "");
    static_assert(2u == decltype(subspan)::extent, "");
    static_assert(1 == subspan[0], "");
    static_assert(2 == subspan[1], "");
  }

  {
    constexpr auto subspan = span.subspan<1, 2>();
    static_assert(span.data() + 1 == subspan.data(), "");
    static_assert(2u == subspan.size(), "");
    static_assert(2u == decltype(subspan)::extent, "");
    static_assert(2 == subspan[0], "");
    static_assert(3 == subspan[1], "");
  }

  {
    constexpr auto subspan = span.subspan<0, 3>();
    static_assert(span.data() == subspan.data(), "");
    static_assert(3u == subspan.size(), "");
    static_assert(3u == decltype(subspan)::extent, "");
    static_assert(1 == subspan[0], "");
    static_assert(2 == subspan[1], "");
    static_assert(3 == subspan[2], "");
  }
}

TEST(SpanTest, SubscriptedBeginIterator) {
  int array[] = {1, 2, 3};
  span<const int> const_span(array);
  for (size_t i = 0; i < const_span.size(); ++i)
    EXPECT_EQ(array[i], const_span.begin()[i]);

  span<int> mutable_span(array);
  for (size_t i = 0; i < mutable_span.size(); ++i)
    EXPECT_EQ(array[i], mutable_span.begin()[i]);
}

TEST(SpanTest, TemplatedFirstOnDynamicSpan) {
  int array[] = {1, 2, 3};
  span<const int> span(array);

  {
    auto subspan = span.first<0>();
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(0u, subspan.size());
    static_assert(0u == decltype(subspan)::extent, "");
  }

  {
    auto subspan = span.first<1>();
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(1u, subspan.size());
    static_assert(1u == decltype(subspan)::extent, "");
    EXPECT_EQ(1, subspan[0]);
  }

  {
    auto subspan = span.first<2>();
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(2u, subspan.size());
    static_assert(2u == decltype(subspan)::extent, "");
    EXPECT_EQ(1, subspan[0]);
    EXPECT_EQ(2, subspan[1]);
  }

  {
    auto subspan = span.first<3>();
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(3u, subspan.size());
    static_assert(3u == decltype(subspan)::extent, "");
    EXPECT_EQ(1, subspan[0]);
    EXPECT_EQ(2, subspan[1]);
    EXPECT_EQ(3, subspan[2]);
  }
}

TEST(SpanTest, TemplatedLastOnDynamicSpan) {
  int array[] = {1, 2, 3};
  span<int> span(array);

  {
    auto subspan = span.last<0>();
    EXPECT_EQ(span.data() + 3, subspan.data());
    EXPECT_EQ(0u, subspan.size());
    static_assert(0u == decltype(subspan)::extent, "");
  }

  {
    auto subspan = span.last<1>();
    EXPECT_EQ(span.data() + 2, subspan.data());
    EXPECT_EQ(1u, subspan.size());
    static_assert(1u == decltype(subspan)::extent, "");
    EXPECT_EQ(3, subspan[0]);
  }

  {
    auto subspan = span.last<2>();
    EXPECT_EQ(span.data() + 1, subspan.data());
    EXPECT_EQ(2u, subspan.size());
    static_assert(2u == decltype(subspan)::extent, "");
    EXPECT_EQ(2, subspan[0]);
    EXPECT_EQ(3, subspan[1]);
  }

  {
    auto subspan = span.last<3>();
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(3u, subspan.size());
    static_assert(3u == decltype(subspan)::extent, "");
    EXPECT_EQ(1, subspan[0]);
    EXPECT_EQ(2, subspan[1]);
    EXPECT_EQ(3, subspan[2]);
  }
}

TEST(SpanTest, TemplatedSubspanFromDynamicSpan) {
  int array[] = {1, 2, 3};
  span<int, 3> span(array);

  {
    auto subspan = span.subspan<0>();
    EXPECT_EQ(span.data(), subspan.data());
    static_assert(3u == decltype(subspan)::extent, "");
    EXPECT_EQ(3u, subspan.size());
    EXPECT_EQ(1, subspan[0]);
    EXPECT_EQ(2, subspan[1]);
    EXPECT_EQ(3, subspan[2]);
  }

  {
    auto subspan = span.subspan<1>();
    EXPECT_EQ(span.data() + 1, subspan.data());
    EXPECT_EQ(2u, subspan.size());
    static_assert(2u == decltype(subspan)::extent, "");
    EXPECT_EQ(2, subspan[0]);
    EXPECT_EQ(3, subspan[1]);
  }

  {
    auto subspan = span.subspan<2>();
    EXPECT_EQ(span.data() + 2, subspan.data());
    EXPECT_EQ(1u, subspan.size());
    static_assert(1u == decltype(subspan)::extent, "");
    EXPECT_EQ(3, subspan[0]);
  }

  {
    auto subspan = span.subspan<3>();
    EXPECT_EQ(span.data() + 3, subspan.data());
    EXPECT_EQ(0u, subspan.size());
    static_assert(0u == decltype(subspan)::extent, "");
  }

  {
    auto subspan = span.subspan<0, 0>();
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(0u, subspan.size());
    static_assert(0u == decltype(subspan)::extent, "");
  }

  {
    auto subspan = span.subspan<1, 0>();
    EXPECT_EQ(span.data() + 1, subspan.data());
    EXPECT_EQ(0u, subspan.size());
    static_assert(0u == decltype(subspan)::extent, "");
  }

  {
    auto subspan = span.subspan<2, 0>();
    EXPECT_EQ(span.data() + 2, subspan.data());
    EXPECT_EQ(0u, subspan.size());
    static_assert(0u == decltype(subspan)::extent, "");
  }

  {
    auto subspan = span.subspan<0, 1>();
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(1u, subspan.size());
    static_assert(1u == decltype(subspan)::extent, "");
    EXPECT_EQ(1, subspan[0]);
  }

  {
    auto subspan = span.subspan<1, 1>();
    EXPECT_EQ(span.data() + 1, subspan.data());
    EXPECT_EQ(1u, subspan.size());
    static_assert(1u == decltype(subspan)::extent, "");
    EXPECT_EQ(2, subspan[0]);
  }

  {
    auto subspan = span.subspan<2, 1>();
    EXPECT_EQ(span.data() + 2, subspan.data());
    EXPECT_EQ(1u, subspan.size());
    static_assert(1u == decltype(subspan)::extent, "");
    EXPECT_EQ(3, subspan[0]);
  }

  {
    auto subspan = span.subspan<0, 2>();
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(2u, subspan.size());
    static_assert(2u == decltype(subspan)::extent, "");
    EXPECT_EQ(1, subspan[0]);
    EXPECT_EQ(2, subspan[1]);
  }

  {
    auto subspan = span.subspan<1, 2>();
    EXPECT_EQ(span.data() + 1, subspan.data());
    EXPECT_EQ(2u, subspan.size());
    static_assert(2u == decltype(subspan)::extent, "");
    EXPECT_EQ(2, subspan[0]);
    EXPECT_EQ(3, subspan[1]);
  }

  {
    auto subspan = span.subspan<0, 3>();
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(3u, subspan.size());
    static_assert(3u == decltype(subspan)::extent, "");
    EXPECT_EQ(1, subspan[0]);
    EXPECT_EQ(2, subspan[1]);
    EXPECT_EQ(3, subspan[2]);
  }
}

TEST(SpanTest, First) {
  int array[] = {1, 2, 3};
  span<int> span(array);

  {
    auto subspan = span.first(0);
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(0u, subspan.size());
  }

  {
    auto subspan = span.first(1);
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(1u, subspan.size());
    EXPECT_EQ(1, subspan[0]);
  }

  {
    auto subspan = span.first(2);
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(2u, subspan.size());
    EXPECT_EQ(1, subspan[0]);
    EXPECT_EQ(2, subspan[1]);
  }

  {
    auto subspan = span.first(3);
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(3u, subspan.size());
    EXPECT_EQ(1, subspan[0]);
    EXPECT_EQ(2, subspan[1]);
    EXPECT_EQ(3, subspan[2]);
  }
}

TEST(SpanTest, Last) {
  int array[] = {1, 2, 3};
  span<int> span(array);

  {
    auto subspan = span.last(0);
    EXPECT_EQ(span.data() + 3, subspan.data());
    EXPECT_EQ(0u, subspan.size());
  }

  {
    auto subspan = span.last(1);
    EXPECT_EQ(span.data() + 2, subspan.data());
    EXPECT_EQ(1u, subspan.size());
    EXPECT_EQ(3, subspan[0]);
  }

  {
    auto subspan = span.last(2);
    EXPECT_EQ(span.data() + 1, subspan.data());
    EXPECT_EQ(2u, subspan.size());
    EXPECT_EQ(2, subspan[0]);
    EXPECT_EQ(3, subspan[1]);
  }

  {
    auto subspan = span.last(3);
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(3u, subspan.size());
    EXPECT_EQ(1, subspan[0]);
    EXPECT_EQ(2, subspan[1]);
    EXPECT_EQ(3, subspan[2]);
  }
}

TEST(SpanTest, Subspan) {
  int array[] = {1, 2, 3};
  span<int> span(array);

  {
    auto subspan = span.subspan(0);
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(3u, subspan.size());
    EXPECT_EQ(1, subspan[0]);
    EXPECT_EQ(2, subspan[1]);
    EXPECT_EQ(3, subspan[2]);
  }

  {
    auto subspan = span.subspan(1);
    EXPECT_EQ(span.data() + 1, subspan.data());
    EXPECT_EQ(2u, subspan.size());
    EXPECT_EQ(2, subspan[0]);
    EXPECT_EQ(3, subspan[1]);
  }

  {
    auto subspan = span.subspan(2);
    EXPECT_EQ(span.data() + 2, subspan.data());
    EXPECT_EQ(1u, subspan.size());
    EXPECT_EQ(3, subspan[0]);
  }

  {
    auto subspan = span.subspan(3);
    EXPECT_EQ(span.data() + 3, subspan.data());
    EXPECT_EQ(0u, subspan.size());
  }

  {
    auto subspan = span.subspan(0, 0);
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(0u, subspan.size());
  }

  {
    auto subspan = span.subspan(1, 0);
    EXPECT_EQ(span.data() + 1, subspan.data());
    EXPECT_EQ(0u, subspan.size());
  }

  {
    auto subspan = span.subspan(2, 0);
    EXPECT_EQ(span.data() + 2, subspan.data());
    EXPECT_EQ(0u, subspan.size());
  }

  {
    auto subspan = span.subspan(0, 1);
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(1u, subspan.size());
    EXPECT_EQ(1, subspan[0]);
  }

  {
    auto subspan = span.subspan(1, 1);
    EXPECT_EQ(span.data() + 1, subspan.data());
    EXPECT_EQ(1u, subspan.size());
    EXPECT_EQ(2, subspan[0]);
  }

  {
    auto subspan = span.subspan(2, 1);
    EXPECT_EQ(span.data() + 2, subspan.data());
    EXPECT_EQ(1u, subspan.size());
    EXPECT_EQ(3, subspan[0]);
  }

  {
    auto subspan = span.subspan(0, 2);
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(2u, subspan.size());
    EXPECT_EQ(1, subspan[0]);
    EXPECT_EQ(2, subspan[1]);
  }

  {
    auto subspan = span.subspan(1, 2);
    EXPECT_EQ(span.data() + 1, subspan.data());
    EXPECT_EQ(2u, subspan.size());
    EXPECT_EQ(2, subspan[0]);
    EXPECT_EQ(3, subspan[1]);
  }

  {
    auto subspan = span.subspan(0, 3);
    EXPECT_EQ(span.data(), subspan.data());
    EXPECT_EQ(span.size(), subspan.size());
    EXPECT_EQ(1, subspan[0]);
    EXPECT_EQ(2, subspan[1]);
    EXPECT_EQ(3, subspan[2]);
  }
}

TEST(SpanTest, Size) {
  {
    span<int> span;
    EXPECT_EQ(0u, span.size());
  }

  {
    int array[] = {1, 2, 3};
    span<int> span(array);
    EXPECT_EQ(3u, span.size());
  }
}

TEST(SpanTest, SizeBytes) {
  {
    span<int> span;
    EXPECT_EQ(0u, span.size_bytes());
  }

  {
    int array[] = {1, 2, 3};
    span<int> span(array);
    EXPECT_EQ(3u * sizeof(int), span.size_bytes());
  }
}

TEST(SpanTest, Empty) {
  {
    span<int> span;
    EXPECT_TRUE(span.empty());
  }

  {
    int array[] = {1, 2, 3};
    span<int> span(array);
    EXPECT_FALSE(span.empty());
  }

  {
    std::vector<int> vector = {1, 2, 3};
    span<int> s = vector;
    span<int> span_of_checked_iterators = {s.end(), s.end()};
    EXPECT_TRUE(span_of_checked_iterators.empty());
  }
}

TEST(SpanTest, OperatorAt) {
  static constexpr int kArray[] = {1, 6, 1, 8, 0};
  constexpr span<const int> span(kArray);

  static_assert(&kArray[0] == &span[0],
                "span[0] does not refer to the same element as kArray[0]");
  static_assert(&kArray[1] == &span[1],
                "span[1] does not refer to the same element as kArray[1]");
  static_assert(&kArray[2] == &span[2],
                "span[2] does not refer to the same element as kArray[2]");
  static_assert(&kArray[3] == &span[3],
                "span[3] does not refer to the same element as kArray[3]");
  static_assert(&kArray[4] == &span[4],
                "span[4] does not refer to the same element as kArray[4]");
}

TEST(SpanTest, Front) {
  static constexpr int kArray[] = {1, 6, 1, 8, 0};
  constexpr span<const int> span(kArray);
  static_assert(&kArray[0] == &span.front(),
                "span.front() does not refer to the same element as kArray[0]");
}

TEST(SpanTest, Back) {
  static constexpr int kArray[] = {1, 6, 1, 8, 0};
  constexpr span<const int> span(kArray);
  static_assert(&kArray[4] == &span.back(),
                "span.back() does not refer to the same element as kArray[4]");
}

TEST(SpanTest, Iterator) {
  static constexpr int kArray[] = {1, 6, 1, 8, 0};
  constexpr span<const int> span(kArray);

  std::vector<int> results;
  for (int i : span)
    results.emplace_back(i);
  EXPECT_THAT(results, ElementsAre(1, 6, 1, 8, 0));
}

TEST(SpanTest, ConstexprIterator) {
  static constexpr int kArray[] = {1, 6, 1, 8, 0};
  constexpr span<const int> span(kArray);

  static_assert(ranges::equal(kArray, span), "");
  static_assert(1 == span.begin()[0], "");
  static_assert(1 == *(span.begin() += 0), "");
  static_assert(6 == *(span.begin() += 1), "");

  static_assert(1 == *((span.begin() + 1) -= 1), "");
  static_assert(6 == *((span.begin() + 1) -= 0), "");

  static_assert(0 + span.begin() == span.begin() + 0);
  static_assert(1 + span.begin() == span.begin() + 1);
}

TEST(SpanTest, ReverseIterator) {
  static constexpr int kArray[] = {1, 6, 1, 8, 0};
  constexpr span<const int> span(kArray);

  EXPECT_TRUE(ranges::equal(Reversed(kArray), Reversed(span)));
}

TEST(SpanTest, AsBytes) {
  {
    constexpr int kArray[] = {2, 3, 5, 7, 11, 13};
    auto bytes_span = as_bytes(make_span(kArray));
    static_assert(std::is_same_v<decltype(bytes_span),
                                 base::span<const uint8_t, sizeof(kArray)>>);
    EXPECT_EQ(reinterpret_cast<const uint8_t*>(kArray), bytes_span.data());
    EXPECT_EQ(sizeof(kArray), bytes_span.size());
    EXPECT_EQ(bytes_span.size(), bytes_span.size_bytes());
  }
  {
    std::vector<int> vec = {1, 1, 2, 3, 5, 8};
    span<int> mutable_span(vec);
    auto bytes_span = as_bytes(mutable_span);
    static_assert(
        std::is_same_v<decltype(bytes_span), base::span<const uint8_t>>);
    EXPECT_EQ(reinterpret_cast<const uint8_t*>(vec.data()), bytes_span.data());
    EXPECT_EQ(sizeof(int) * vec.size(), bytes_span.size());
    EXPECT_EQ(bytes_span.size(), bytes_span.size_bytes());
  }
}

TEST(SpanTest, AsWritableBytes) {
  std::vector<int> vec = {1, 1, 2, 3, 5, 8};
  span<int> mutable_span(vec);
  auto writable_bytes_span = as_writable_bytes(mutable_span);
  static_assert(
      std::is_same_v<decltype(writable_bytes_span), base::span<uint8_t>>);
  EXPECT_EQ(reinterpret_cast<uint8_t*>(vec.data()), writable_bytes_span.data());
  EXPECT_EQ(sizeof(int) * vec.size(), writable_bytes_span.size());
  EXPECT_EQ(writable_bytes_span.size(), writable_bytes_span.size_bytes());

  // Set the first entry of vec by writing through the span.
  std::fill(writable_bytes_span.data(),
            writable_bytes_span.data() + sizeof(int), 'a');
  static_assert(sizeof(int) == 4u);  // Otherwise char literal wrong below.
  EXPECT_EQ('aaaa', vec[0]);
}

TEST(SpanTest, AsChars) {
  {
    constexpr int kArray[] = {2, 3, 5, 7, 11, 13};
    auto chars_span = as_chars(make_span(kArray));
    static_assert(std::is_same_v<decltype(chars_span),
                                 base::span<const char, sizeof(kArray)>>);
    EXPECT_EQ(reinterpret_cast<const char*>(kArray), chars_span.data());
    EXPECT_EQ(sizeof(kArray), chars_span.size());
    EXPECT_EQ(chars_span.size(), chars_span.size_bytes());
  }
  {
    std::vector<int> vec = {1, 1, 2, 3, 5, 8};
    span<int> mutable_span(vec);
    auto chars_span = as_chars(mutable_span);
    static_assert(std::is_same_v<decltype(chars_span), base::span<const char>>);
    EXPECT_EQ(reinterpret_cast<const char*>(vec.data()), chars_span.data());
    EXPECT_EQ(sizeof(int) * vec.size(), chars_span.size());
    EXPECT_EQ(chars_span.size(), chars_span.size_bytes());
  }
}

TEST(SpanTest, AsWritableChars) {
  std::vector<int> vec = {1, 1, 2, 3, 5, 8};
  span<int> mutable_span(vec);
  auto writable_chars_span = as_writable_chars(mutable_span);
  static_assert(
      std::is_same_v<decltype(writable_chars_span), base::span<char>>);
  EXPECT_EQ(reinterpret_cast<char*>(vec.data()), writable_chars_span.data());
  EXPECT_EQ(sizeof(int) * vec.size(), writable_chars_span.size());
  EXPECT_EQ(writable_chars_span.size(), writable_chars_span.size_bytes());

  // Set the first entry of vec by writing through the span.
  std::fill(writable_chars_span.data(),
            writable_chars_span.data() + sizeof(int), 'a');
  static_assert(sizeof(int) == 4u);  // Otherwise char literal wrong below.
  EXPECT_EQ('aaaa', vec[0]);
}

TEST(SpanTest, AsByteSpan) {
  {
    constexpr int kArray[] = {2, 3, 5, 7, 11, 13};
    auto byte_span = as_byte_span(kArray);
    static_assert(std::is_same_v<decltype(byte_span), span<const uint8_t>>);
    EXPECT_EQ(byte_span.data(), reinterpret_cast<const uint8_t*>(kArray));
    EXPECT_EQ(byte_span.size(), sizeof(kArray));
  }
  {
    int kMutArray[] = {2, 3, 5, 7};
    auto byte_span = as_byte_span(kMutArray);
    static_assert(std::is_same_v<decltype(byte_span), span<const uint8_t>>);
    EXPECT_EQ(byte_span.data(), reinterpret_cast<const uint8_t*>(kMutArray));
    EXPECT_EQ(byte_span.size(), sizeof(kMutArray));
  }
}

TEST(SpanTest, MakeSpanFromDataAndSize) {
  int* nullint = nullptr;
  auto empty_span = make_span(nullint, 0u);
  EXPECT_TRUE(empty_span.empty());
  EXPECT_EQ(nullptr, empty_span.data());

  std::vector<int> vector = {1, 1, 2, 3, 5, 8};
  span<int> expected_span(vector.data(), vector.size());
  auto made_span = make_span(vector.data(), vector.size());
  EXPECT_EQ(expected_span.data(), made_span.data());
  EXPECT_EQ(expected_span.size(), made_span.size());
  static_assert(decltype(made_span)::extent == dynamic_extent, "");
  static_assert(std::is_same_v<decltype(expected_span), decltype(made_span)>,
                "the type of made_span differs from expected_span!");
}

TEST(SpanTest, MakeSpanFromPointerPair) {
  int* nullint = nullptr;
  auto empty_span = make_span(nullint, nullint);
  EXPECT_TRUE(empty_span.empty());
  EXPECT_EQ(nullptr, empty_span.data());

  std::vector<int> vector = {1, 1, 2, 3, 5, 8};
  span<int> expected_span(vector.data(), vector.size());
  auto made_span = make_span(vector.data(), vector.data() + vector.size());
  EXPECT_EQ(expected_span.data(), made_span.data());
  EXPECT_EQ(expected_span.size(), made_span.size());
  static_assert(decltype(made_span)::extent == dynamic_extent, "");
  static_assert(std::is_same_v<decltype(expected_span), decltype(made_span)>,
                "the type of made_span differs from expected_span!");
}

TEST(SpanTest, MakeSpanFromConstexprArray) {
  static constexpr int kArray[] = {1, 2, 3, 4, 5};
  constexpr span<const int, 5> expected_span(kArray);
  constexpr auto made_span = make_span(kArray);
  EXPECT_EQ(expected_span.data(), made_span.data());
  EXPECT_EQ(expected_span.size(), made_span.size());
  static_assert(decltype(made_span)::extent == 5, "");
  static_assert(std::is_same_v<decltype(expected_span), decltype(made_span)>,
                "the type of made_span differs from expected_span!");
}

TEST(SpanTest, MakeSpanFromStdArray) {
  const std::array<int, 5> kArray = {{1, 2, 3, 4, 5}};
  span<const int, 5> expected_span(kArray);
  auto made_span = make_span(kArray);
  EXPECT_EQ(expected_span.data(), made_span.data());
  EXPECT_EQ(expected_span.size(), made_span.size());
  static_assert(decltype(made_span)::extent == 5, "");
  static_assert(std::is_same_v<decltype(expected_span), decltype(made_span)>,
                "the type of made_span differs from expected_span!");
}

TEST(SpanTest, MakeSpanFromConstContainer) {
  const std::vector<int> vector = {-1, -2, -3, -4, -5};
  span<const int> expected_span(vector);
  auto made_span = make_span(vector);
  EXPECT_EQ(expected_span.data(), made_span.data());
  EXPECT_EQ(expected_span.size(), made_span.size());
  static_assert(decltype(made_span)::extent == dynamic_extent, "");
  static_assert(std::is_same_v<decltype(expected_span), decltype(made_span)>,
                "the type of made_span differs from expected_span!");
}

TEST(SpanTest, MakeStaticSpanFromConstContainer) {
  const std::vector<int> vector = {-1, -2, -3, -4, -5};
  span<const int, 5> expected_span(vector.data(), vector.size());
  auto made_span = make_span<5>(vector);
  EXPECT_EQ(expected_span.data(), made_span.data());
  EXPECT_EQ(expected_span.size(), made_span.size());
  static_assert(decltype(made_span)::extent == 5, "");
  static_assert(std::is_same_v<decltype(expected_span), decltype(made_span)>,
                "the type of made_span differs from expected_span!");
}

TEST(SpanTest, MakeSpanFromContainer) {
  std::vector<int> vector = {-1, -2, -3, -4, -5};
  span<int> expected_span(vector);
  auto made_span = make_span(vector);
  EXPECT_EQ(expected_span.data(), made_span.data());
  EXPECT_EQ(expected_span.size(), made_span.size());
  static_assert(decltype(made_span)::extent == dynamic_extent, "");
  static_assert(std::is_same_v<decltype(expected_span), decltype(made_span)>,
                "the type of made_span differs from expected_span!");
}

TEST(SpanTest, MakeStaticSpanFromContainer) {
  std::vector<int> vector = {-1, -2, -3, -4, -5};
  span<int, 5> expected_span(vector.data(), vector.size());
  auto made_span = make_span<5>(vector);
  EXPECT_EQ(expected_span.data(), make_span<5>(vector).data());
  EXPECT_EQ(expected_span.size(), make_span<5>(vector).size());
  static_assert(decltype(make_span<5>(vector))::extent == 5, "");
  static_assert(std::is_same_v<decltype(expected_span), decltype(made_span)>,
                "the type of made_span differs from expected_span!");
}

TEST(SpanTest, MakeStaticSpanFromConstexprContainer) {
  constexpr StringPiece str = "Hello, World";
  constexpr auto made_span = make_span<12>(str);
  static_assert(str.data() == made_span.data(), "Error: data() does not match");
  static_assert(str.size() == made_span.size(), "Error: size() does not match");
  static_assert(std::is_same_v<decltype(str)::value_type,
                               decltype(made_span)::value_type>,
                "Error: value_type does not match");
  static_assert(str.size() == decltype(made_span)::extent,
                "Error: extent does not match");
}

TEST(SpanTest, MakeSpanFromRValueContainer) {
  std::vector<int> vector = {-1, -2, -3, -4, -5};
  span<const int> expected_span(vector);
  // Note: While static_cast<T&&>(foo) is effectively just a fancy spelling of
  // std::move(foo), make_span does not actually take ownership of the passed in
  // container. Writing it this way makes it more obvious that we simply care
  // about the right behavour when passing rvalues.
  auto made_span = make_span(static_cast<std::vector<int>&&>(vector));
  EXPECT_EQ(expected_span.data(), made_span.data());
  EXPECT_EQ(expected_span.size(), made_span.size());
  static_assert(decltype(made_span)::extent == dynamic_extent, "");
  static_assert(std::is_same_v<decltype(expected_span), decltype(made_span)>,
                "the type of made_span differs from expected_span!");
}

TEST(SpanTest, MakeStaticSpanFromRValueContainer) {
  std::vector<int> vector = {-1, -2, -3, -4, -5};
  span<const int, 5> expected_span(vector.data(), vector.size());
  // Note: While static_cast<T&&>(foo) is effectively just a fancy spelling of
  // std::move(foo), make_span does not actually take ownership of the passed in
  // container. Writing it this way makes it more obvious that we simply care
  // about the right behavour when passing rvalues.
  auto made_span = make_span<5>(static_cast<std::vector<int>&&>(vector));
  EXPECT_EQ(expected_span.data(), made_span.data());
  EXPECT_EQ(expected_span.size(), made_span.size());
  static_assert(decltype(made_span)::extent == 5, "");
  static_assert(std::is_same_v<decltype(expected_span), decltype(made_span)>,
                "the type of made_span differs from expected_span!");
}

TEST(SpanTest, MakeSpanFromDynamicSpan) {
  static constexpr int kArray[] = {1, 2, 3, 4, 5};
  constexpr span<const int> expected_span(kArray);
  constexpr auto made_span = make_span(expected_span);
  static_assert(std::is_same_v<decltype(expected_span)::element_type,
                               decltype(made_span)::element_type>,
                "make_span(span) should have the same element_type as span");

  static_assert(expected_span.data() == made_span.data(),
                "make_span(span) should have the same data() as span");

  static_assert(expected_span.size() == made_span.size(),
                "make_span(span) should have the same size() as span");

  static_assert(decltype(made_span)::extent == decltype(expected_span)::extent,
                "make_span(span) should have the same extent as span");

  static_assert(std::is_same_v<decltype(expected_span), decltype(made_span)>,
                "the type of made_span differs from expected_span!");
}

TEST(SpanTest, MakeSpanFromStaticSpan) {
  static constexpr int kArray[] = {1, 2, 3, 4, 5};
  constexpr span<const int, 5> expected_span(kArray);
  constexpr auto made_span = make_span(expected_span);
  static_assert(std::is_same_v<decltype(expected_span)::element_type,
                               decltype(made_span)::element_type>,
                "make_span(span) should have the same element_type as span");

  static_assert(expected_span.data() == made_span.data(),
                "make_span(span) should have the same data() as span");

  static_assert(expected_span.size() == made_span.size(),
                "make_span(span) should have the same size() as span");

  static_assert(decltype(made_span)::extent == decltype(expected_span)::extent,
                "make_span(span) should have the same extent as span");

  static_assert(std::is_same_v<decltype(expected_span), decltype(made_span)>,
                "the type of made_span differs from expected_span!");
}

TEST(SpanTest, EnsureConstexprGoodness) {
  static constexpr int kArray[] = {5, 4, 3, 2, 1};
  constexpr span<const int> constexpr_span(kArray);
  const size_t size = 2;

  const size_t start = 1;
  constexpr span<const int> subspan =
      constexpr_span.subspan(start, start + size);
  for (size_t i = 0; i < subspan.size(); ++i)
    EXPECT_EQ(kArray[start + i], subspan[i]);

  constexpr span<const int> firsts = constexpr_span.first(size);
  for (size_t i = 0; i < firsts.size(); ++i)
    EXPECT_EQ(kArray[i], firsts[i]);

  constexpr span<const int> lasts = constexpr_span.last(size);
  for (size_t i = 0; i < lasts.size(); ++i) {
    const size_t j = (std::size(kArray) - size) + i;
    EXPECT_EQ(kArray[j], lasts[i]);
  }

  constexpr int item = constexpr_span[size];
  EXPECT_EQ(kArray[size], item);
}

TEST(SpanTest, OutOfBoundsDeath) {
  constexpr span<int, 0> kEmptySpan;
  ASSERT_DEATH_IF_SUPPORTED(kEmptySpan[0], "");
  ASSERT_DEATH_IF_SUPPORTED(kEmptySpan.first(1), "");
  ASSERT_DEATH_IF_SUPPORTED(kEmptySpan.last(1), "");
  ASSERT_DEATH_IF_SUPPORTED(kEmptySpan.subspan(1), "");

  constexpr span<int> kEmptyDynamicSpan;
  ASSERT_DEATH_IF_SUPPORTED(kEmptyDynamicSpan[0], "");
  ASSERT_DEATH_IF_SUPPORTED(kEmptyDynamicSpan.front(), "");
  ASSERT_DEATH_IF_SUPPORTED(kEmptyDynamicSpan.first(1), "");
  ASSERT_DEATH_IF_SUPPORTED(kEmptyDynamicSpan.last(1), "");
  ASSERT_DEATH_IF_SUPPORTED(kEmptyDynamicSpan.back(), "");
  ASSERT_DEATH_IF_SUPPORTED(kEmptyDynamicSpan.subspan(1), "");

  static constexpr int kArray[] = {0, 1, 2};
  constexpr span<const int> kNonEmptyDynamicSpan(kArray);
  EXPECT_EQ(3U, kNonEmptyDynamicSpan.size());
  ASSERT_DEATH_IF_SUPPORTED(kNonEmptyDynamicSpan[4], "");
  ASSERT_DEATH_IF_SUPPORTED(kNonEmptyDynamicSpan.subspan(10), "");
  ASSERT_DEATH_IF_SUPPORTED(kNonEmptyDynamicSpan.subspan(1, 7), "");

  size_t minus_one = static_cast<size_t>(-1);
  ASSERT_DEATH_IF_SUPPORTED(kNonEmptyDynamicSpan.subspan(minus_one), "");
  ASSERT_DEATH_IF_SUPPORTED(kNonEmptyDynamicSpan.subspan(minus_one, minus_one),
                            "");
  ASSERT_DEATH_IF_SUPPORTED(kNonEmptyDynamicSpan.subspan(minus_one, 1), "");

  // Span's iterators should be checked. To confirm the crashes come from the
  // iterator checks and not stray memory accesses, we create spans that are
  // backed by larger arrays.
  int array1[] = {1, 2, 3, 4};
  int array2[] = {1, 2, 3, 4};
  span<int> span_len2 = span(array1).first(2);
  span<int> span_len3 = span(array2).first(3);
  ASSERT_DEATH_IF_SUPPORTED(*span_len2.end(), "");
  ASSERT_DEATH_IF_SUPPORTED(span_len2.begin()[2], "");
  ASSERT_DEATH_IF_SUPPORTED(span_len2.begin() + 3, "");
  ASSERT_DEATH_IF_SUPPORTED(span_len2.begin() - 1, "");
  ASSERT_DEATH_IF_SUPPORTED(span_len2.end() + 1, "");

  // When STL functions take explicit end iterators, bounds checking happens
  // at the caller, when end iterator is created. However, some APIs take only a
  // begin iterator and determine end implicitly. In that case, bounds checking
  // happens inside the STL. However, the STL sometimes specializes operations
  // on contiguous iterators. These death ensures this specialization does not
  // lose hardening.
  //
  // Note that these tests are necessary, but not sufficient, to demonstrate
  // that iterators are suitably checked. The output iterator is currently
  // checked too late due to https://crbug.com/1520041.

  // Copying more values than fit in the destination.
  ASSERT_DEATH_IF_SUPPORTED(
      std::copy(span_len3.begin(), span_len3.end(), span_len2.begin()), "");
  ASSERT_DEATH_IF_SUPPORTED(std::ranges::copy(span_len3, span_len2.begin()),
                            "");
  ASSERT_DEATH_IF_SUPPORTED(
      std::copy_n(span_len3.begin(), 3, span_len2.begin()), "");

  // Copying more values than exist in the source.
  ASSERT_DEATH_IF_SUPPORTED(
      std::copy_n(span_len2.begin(), 3, span_len3.begin()), "");
}

TEST(SpanTest, IteratorIsRangeMoveSafe) {
  static constexpr int kArray[] = {1, 6, 1, 8, 0};
  const size_t kNumElements = 5;
  constexpr span<const int> span(kArray);

  static constexpr int kOverlappingStartIndexes[] = {-4, 0, 3, 4};
  static constexpr int kNonOverlappingStartIndexes[] = {-7, -5, 5, 7};

  // Overlapping ranges.
  for (const int dest_start_index : kOverlappingStartIndexes) {
    EXPECT_FALSE(CheckedContiguousIterator<const int>::IsRangeMoveSafe(
        span.begin(), span.end(),
        CheckedContiguousIterator<const int>(
            span.data() + dest_start_index,
            span.data() + dest_start_index + kNumElements)));
  }

  // Non-overlapping ranges.
  for (const int dest_start_index : kNonOverlappingStartIndexes) {
    EXPECT_TRUE(CheckedContiguousIterator<const int>::IsRangeMoveSafe(
        span.begin(), span.end(),
        CheckedContiguousIterator<const int>(
            span.data() + dest_start_index,
            span.data() + dest_start_index + kNumElements)));
  }

  // IsRangeMoveSafe is true if the length to be moved is 0.
  EXPECT_TRUE(CheckedContiguousIterator<const int>::IsRangeMoveSafe(
      span.begin(), span.begin(),
      CheckedContiguousIterator<const int>(span.data(), span.data())));

  // IsRangeMoveSafe is false if end < begin.
  EXPECT_FALSE(CheckedContiguousIterator<const int>::IsRangeMoveSafe(
      span.end(), span.begin(),
      CheckedContiguousIterator<const int>(span.data(), span.data())));
}

TEST(SpanTest, Sort) {
  int array[] = {5, 4, 3, 2, 1};

  span<int> dynamic_span = array;
  ranges::sort(dynamic_span);
  EXPECT_THAT(array, ElementsAre(1, 2, 3, 4, 5));
  std::sort(dynamic_span.rbegin(), dynamic_span.rend());
  EXPECT_THAT(array, ElementsAre(5, 4, 3, 2, 1));

  span<int, 5> static_span = array;
  std::sort(static_span.rbegin(), static_span.rend(), std::greater<>());
  EXPECT_THAT(array, ElementsAre(1, 2, 3, 4, 5));
  ranges::sort(static_span, std::greater<>());
  EXPECT_THAT(array, ElementsAre(5, 4, 3, 2, 1));
}

TEST(SpanTest, SpanExtentConversions) {
  // Statically checks that various conversions between spans of dynamic and
  // static extent are possible or not.
  static_assert(std::is_constructible_v<span<int, 0>, span<int>>,
                "Error: static span should be constructible from dynamic span");

  static_assert(
      !std::is_convertible_v<span<int>, span<int, 0>>,
      "Error: static span should not be convertible from dynamic span");

  static_assert(!std::is_constructible_v<span<int, 2>, span<int, 1>>,
                "Error: static span should not be constructible from static "
                "span with different extent");

  static_assert(std::is_convertible_v<span<int, 0>, span<int>>,
                "Error: static span should be convertible to dynamic span");

  static_assert(std::is_convertible_v<span<int>, span<int>>,
                "Error: dynamic span should be convertible to dynamic span");

  static_assert(std::is_convertible_v<span<int, 2>, span<int, 2>>,
                "Error: static span should be convertible to static span");
}

TEST(SpanTest, IteratorConversions) {
  static_assert(
      std::is_convertible_v<span<int>::iterator, span<const int>::iterator>,
      "Error: iterator should be convertible to const iterator");

  static_assert(
      !std::is_convertible_v<span<const int>::iterator, span<int>::iterator>,
      "Error: const iterator should not be convertible to iterator");
}

TEST(SpanTest, ExtentMacro) {
  constexpr size_t kSize = 10;
  std::array<uint8_t, kSize> array;
  static_assert(EXTENT(array) == kSize, "EXTENT broken");

  const std::array<uint8_t, kSize>& reference = array;
  static_assert(EXTENT(reference) == kSize, "EXTENT broken for references");

  const std::array<uint8_t, kSize>* pointer = nullptr;
  static_assert(EXTENT(*pointer) == kSize, "EXTENT broken for pointers");

  uint8_t plain_array[kSize] = {0};
  static_assert(EXTENT(plain_array) == kSize, "EXTENT broken for plain arrays");
}

TEST(SpanTest, CopyFrom) {
  int arr[] = {1, 2, 3};
  span<int, 0> empty_static_span;
  span<int, 3> static_span = base::make_span(arr);

  std::vector<int> vec = {4, 5, 6};
  span<int> empty_dynamic_span;
  span<int> dynamic_span = base::make_span(vec);

  // Handle empty cases gracefully.
  empty_static_span.copy_from(empty_dynamic_span);
  empty_dynamic_span.copy_from(empty_static_span);
  static_span.first(empty_static_span.size()).copy_from(empty_static_span);
  dynamic_span.first(empty_dynamic_span.size()).copy_from(empty_dynamic_span);
  EXPECT_THAT(arr, ElementsAre(1, 2, 3));
  EXPECT_THAT(vec, ElementsAre(4, 5, 6));

  // Test too small destinations.
  EXPECT_DEATH_IF_SUPPORTED(empty_static_span.copy_from(dynamic_span), "");
  EXPECT_DEATH_IF_SUPPORTED(empty_dynamic_span.copy_from(static_span), "");
  EXPECT_DEATH_IF_SUPPORTED(empty_dynamic_span.copy_from(dynamic_span), "");
  EXPECT_DEATH_IF_SUPPORTED(static_span.first(2).copy_from(dynamic_span), "");
  EXPECT_DEATH_IF_SUPPORTED(dynamic_span.last(2).copy_from(static_span), "");

  static_span.first(2).copy_from(static_span.last(2));
  EXPECT_THAT(arr, ElementsAre(2, 3, 3));

  dynamic_span.first(2).copy_from(dynamic_span.last(2));
  EXPECT_THAT(vec, ElementsAre(5, 6, 6));

  static_span.last(1).copy_from(dynamic_span.last(1));
  EXPECT_THAT(arr, ElementsAre(2, 3, 6));

  dynamic_span.first(1).copy_from(static_span.first(1));
  EXPECT_THAT(vec, ElementsAre(2, 6, 6));
}

}  // namespace base
