// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <array>
#include <cstdint>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace taktile {
static constexpr const char* VERSION = "0.0.0";

enum class Scheme {
  HTTP,
  HTTPS,
  TLS,
  TCP,
  UDP,
  UDP_BROADCAST,
  UDP_WRITE_ONLY,
  LOG
};

static const std::unordered_map<Scheme, std::string> SCHEME_FWD_MAP{
    {Scheme::HTTP, "http"},
    {Scheme::HTTPS, "https"},
    {Scheme::TLS, "tls"},
    {Scheme::TCP, "tcp"},
    {Scheme::UDP, "udp"},
    {Scheme::UDP_BROADCAST, "udp+broadcast"},
    {Scheme::UDP_WRITE_ONLY, "udp+wo"},
    {Scheme::LOG, "log"}};  // NOLINT[whitespace/indent_namespace]

static const std::unordered_map<std::string, Scheme> SCHEME_INV_MAP{
    {"http", Scheme::HTTP},
    {"https", Scheme::HTTPS},
    {"tls", Scheme::TLS},
    {"tcp", Scheme::TCP},
    {"udp", Scheme::UDP},
    {"udp+broadcast", Scheme::UDP_BROADCAST},
    {"udp+wo", Scheme::UDP_WRITE_ONLY},
    {"log", Scheme::LOG}};  // NOLINT[whitespace/indent_namespace]

// Helper for C-style string literal length (excluding null terminator)
constexpr size_t cstr_len(const char* c_str) {
    return sizeof(c_str) - 1;
}

static constexpr const char* DEFAULT_IPV4_ADDRESS{"239.2.3.1"};
static constexpr uint16_t DEFAULT_BROADCAST_PORT{6969};
static constexpr uint16_t DEFAULT_COT_PORT{8087};
static constexpr uint64_t DEFAULT_COT_STALE{120000};  // milliseconds
static constexpr double DEFAULT_COT_VAL{9999999.0};
extern const std::string DEFAULT_HOST_ID;
static constexpr const char* DEFAULT_COT_HOW{"m-g"};
static constexpr const char* DEFAULT_COT_TYPE{"a-u-G"};
static constexpr const char* W3C_XML_DATETIME{"{:%Y-%m-%dT%H:%M:%S}.{:03d}Z"};
static std::regex const W3C_XML_DATETIME_REGEX(
    R"(^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d{3}Z$)");
static constexpr size_t MAX_UDP_BLOB_SIZE{1400};   // # of bytes
static constexpr size_t MAX_TCP_BLOB_SIZE{64000};  // # of bytes
static constexpr double LATITUDE_BOUND{90.0};      // degrees
static constexpr double LONGITUDE_BOUND{180.0};    // degrees
static constexpr std::string_view V0_PROTOCOL_PREFIX{R"(<?xml version="1.0" encoding="UTF-8"?>)"};
static constexpr std::string_view V0_PROTOCOL_SUFFIX{"</event>"};
static constexpr const char V1_PROTOCOL_MAGIC = static_cast<const char>(0xBF);
// NOLINTBEGIN(whitespace/indent_namespace)
static constexpr const char V1_MESH_PROTOCOL_PREFIX_ARR[] = {
    V1_PROTOCOL_MAGIC, 0x01, V1_PROTOCOL_MAGIC};
static const std::string_view V1_MESH_PROTOCOL_PREFIX{
    V1_MESH_PROTOCOL_PREFIX_ARR, sizeof(V1_MESH_PROTOCOL_PREFIX_ARR)};
static constexpr size_t V1_PROTOCOL_MAX_VARINT_SIZE{
    10};  // From protobuf spec:
          // https://protobuf.dev/programming-guides/encoding/#varints, see
          // "Base 128 Varints" section
// NOLINTEND(whitespace/indent_namespace)
}  // namespace taktile
