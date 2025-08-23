// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <optional>
#include <string>

namespace taktile {
void init_logger();

class TimeProvider {
 public:
  /// @brief Get the current time since epoch in milliseconds
  /// @details Comparable to cot_time
  /// @param cot_stale time in milliseconds before the message is considered
  /// stale (optional)
  /// @return time since epoch in milliseconds
  static uint64_t get_time(std::optional<uint64_t> cot_stale = std::nullopt);
  static std::string to_datetime(uint64_t milliseconds);
  static std::optional<uint64_t> from_datetime(std::string const& datetime);
};

class Varint {
 public:
  static constexpr auto CONTINUE_BIT = 0x80U;
  static constexpr auto BIT_MASK = 0x7FU;
  static constexpr auto VARINT_SIZE_BITS = 64;
  static constexpr auto PAYLOAD_BITS_PER_BYTE = 7;
  
  // Write unsigned varint (protobuf-compatible)
  static std::string encode(uint64_t payload_length);

  struct DecodeResult {
    uint64_t payload_length;
    size_t bytes_used;
  };
  static DecodeResult decode(std::string const& blob);
};

}  // namespace taktile
