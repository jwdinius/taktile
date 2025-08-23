// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#include <fmt/core.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <regex>
#include <simpleio/messages/xml.hpp>
#include <string>
#include <utility>

#include "taktile/constants.hpp"
#include "taktile/functions.hpp"

TEST(Functions, TimeProvider_ToFromDatetime) {
  /// Test that get_time returns the current system time in W3C XML datetime
  auto const time_msec = taktile::TimeProvider::get_time();
  auto const datetime = taktile::TimeProvider::to_datetime(time_msec);
  EXPECT_TRUE(std::regex_match(datetime, taktile::W3C_XML_DATETIME_REGEX));
  auto msec = taktile::TimeProvider::from_datetime(datetime);
  EXPECT_TRUE(msec.has_value());
  EXPECT_EQ(msec.value(), time_msec);
}

constexpr uint64_t varint_min_for_bytes(size_t n) {
  return (n == 1) ? 0 : (1ULL << (7 * (n - 1)));
}

constexpr uint64_t varint_max_for_bytes(size_t n) {
  return (1ULL << (7 * n)) - 1;
}

constexpr std::array<uint64_t, 10> evenly_distributed_offsets(uint64_t min,
                                                              uint64_t max) {
  std::array<uint64_t, 10> result{};
  for (size_t i = 0; i < 10; ++i) {
    double fraction = static_cast<double>(i) / 10;
    result[i] = static_cast<uint64_t>(min + fraction * (max - min));
  }
  return result;
}

auto const encode_decode_helper(size_t num_bytes) {
  auto const min = varint_min_for_bytes(num_bytes);
  auto const max = varint_max_for_bytes(num_bytes);
  auto const offsets = evenly_distributed_offsets(min, max);
  for (size_t i = 0; i < offsets.size(); ++i) {
    auto const& value = offsets[i];
    std::string encoded = taktile::Varint::encode(value);
    EXPECT_EQ(encoded.size(), num_bytes);
    auto decoded = taktile::Varint::decode(encoded);
    EXPECT_EQ(decoded.payload_length, value);
    EXPECT_EQ(decoded.bytes_used, encoded.size());
  }
}

TEST(Functions, Varint_EncodeDecodeValues) {
  for (size_t i = 1; i <= taktile::V1_PROTOCOL_MAX_VARINT_SIZE; ++i) {
    encode_decode_helper(i);
  }
}
TEST(Functions, Varint_EncodeDecodeMax64Bit) {
  uint64_t input = std::numeric_limits<uint64_t>::max();
  std::string encoded = taktile::Varint::encode(input);
  EXPECT_EQ(encoded.size(), taktile::V1_PROTOCOL_MAX_VARINT_SIZE);
  auto decoding = taktile::Varint::decode(encoded);
  EXPECT_EQ(decoding.payload_length, input);
  EXPECT_EQ(decoding.bytes_used, encoded.size());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
