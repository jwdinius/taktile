// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#include "taktile/functions.hpp"

#include <fmt/chrono.h>
#include <fmt/core.h>

#include <boost/log/attributes/clock.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>

#include "simpleio/message.hpp"
#include "taktile/constants.hpp"

namespace taktile {
void init_logger() {
  boost::log::core::get()->add_global_attribute(
      "TimeStamp", boost::log::attributes::local_clock());
  // Set up the logger
  boost::log::add_console_log(
      std::clog,
      boost::log::keywords::format = "[%TimeStamp%] [%Severity%] %Message%");
  // boost::log::add_file_log("taktile.log", boost::log::keywords::format =
  // "[%TimeStamp%] [%Severity%] %Message%");
  boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                      boost::log::trivial::debug);
}

std::string TimeProvider::to_datetime(uint64_t milliseconds) {
  auto tp_ms = std::chrono::time_point<std::chrono::system_clock,
                                       std::chrono::milliseconds>(
      std::chrono::milliseconds(milliseconds));
  auto const seconds =
      std::chrono::time_point_cast<std::chrono::seconds>(tp_ms);
  auto const msec =
      std::chrono::duration_cast<std::chrono::milliseconds>((tp_ms - seconds));
  return fmt::format(W3C_XML_DATETIME, seconds, msec.count());
}

std::optional<uint64_t> TimeProvider::from_datetime(
    std::string const& datetime) {
  static constexpr size_t DATETIME_COMPONENTS{7};
  static constexpr int MIN_YEAR{1900};
  int millis;
  std::tm dtc = {};
  if (sscanf(datetime.c_str(), "%d-%d-%dT%d:%d:%d.%dZ", &dtc.tm_year,
             &dtc.tm_mon, &dtc.tm_mday, &dtc.tm_hour, &dtc.tm_min, &dtc.tm_sec,
             &millis) < DATETIME_COMPONENTS - 1) {
    BOOST_LOG_TRIVIAL(error)
        << "Could not parse datetime: " << datetime << std::endl;
    return std::nullopt;
  }

  dtc.tm_year -= MIN_YEAR;
  dtc.tm_mon -= 1;

  time_t t_utc = timegm(&dtc);  // UTC
  if (t_utc == -1) {
    return std::nullopt;
  }

  uint64_t base_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::from_time_t(t_utc).time_since_epoch())
          .count();
  return base_ms + millis;
}

/// @brief Static methods in TakData
uint64_t TimeProvider::get_time(std::optional<uint64_t> cot_stale) {
  auto time = std::chrono::system_clock::now();
  if (cot_stale.has_value()) {
    time += std::chrono::milliseconds(cot_stale.value());
  }
  auto const milliseconds =
      std::chrono::time_point_cast<std::chrono::milliseconds>(time);
  return milliseconds.time_since_epoch().count();
}

std::string Varint::encode(std::uint64_t payload_length) {
  std::string out;
  while (payload_length >= Varint::CONTINUE_BIT) {
    out.push_back(static_cast<char>((payload_length & Varint::BIT_MASK) |
                                    Varint::CONTINUE_BIT));
    payload_length >>= Varint::PAYLOAD_BITS_PER_BYTE;
  }
  out.push_back(static_cast<char>(payload_length & Varint::BIT_MASK));
  return out;
}

taktile::Varint::DecodeResult Varint::decode(std::string const& blob) {
  uint64_t result = 0;
  int32_t shift = 0;
  size_t bytes_used = 0;
  const auto* const front = blob.data();
  while (bytes_used < blob.size()) {
    auto const character = static_cast<uint8_t>(front[bytes_used++]);
    uint64_t chunk = (character & Varint::BIT_MASK);
    if (shift >= Varint::VARINT_SIZE_BITS ||
        (chunk << shift >> shift) != chunk) {
      throw simpleio::SerializerError("varint overflow");
    }
    result |= (chunk << shift);
    if ((character & Varint::CONTINUE_BIT) == 0) {
      return {result, bytes_used};
    }
    shift += Varint::PAYLOAD_BITS_PER_BYTE;
    if (bytes_used > taktile::V1_PROTOCOL_MAX_VARINT_SIZE) {
      std::stringstream s_str;
      s_str << "varint too long: " << bytes_used;
      throw simpleio::SerializerError(s_str.str());
    }
  }
  throw simpleio::SerializerError("incomplete varint");
}

}  // namespace taktile
