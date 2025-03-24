// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

namespace taktile {
static constexpr std::string_view VERSION = "0.0.0";

enum class Scheme { HTTPS, TLS, TCP, UDP, UDP_BROADCAST, UDP_WRITE_ONLY, LOG };

static const std::unordered_map<Scheme, std::string> SCHEME_FWD_MAP{
    {Scheme::HTTPS, "https"},
    {Scheme::TLS, "tls"},
    {Scheme::TCP, "tcp"},
    {Scheme::UDP, "udp"},
    {Scheme::UDP_BROADCAST, "udp+broadcast"},
    {Scheme::UDP_WRITE_ONLY, "udp+wo"},
    {Scheme::LOG, "log"}};  // NOLINT[whitespace/indent_namespace]

static const std::unordered_map<std::string, Scheme> SCHEME_INV_MAP{
    {"https", Scheme::HTTPS},
    {"tls", Scheme::TLS},
    {"tcp", Scheme::TCP},
    {"udp", Scheme::UDP},
    {"udp+broadcast", Scheme::UDP_BROADCAST},
    {"udp+wo", Scheme::UDP_WRITE_ONLY},
    {"log", Scheme::LOG}};  // NOLINT[whitespace/indent_namespace]

static const char* const DEFAULT_IPV4_ADDRESS{"239.2.3.1"};
static constexpr uint16_t DEFAULT_BROADCAST_PORT{6969};
static constexpr uint16_t DEFAULT_COT_PORT{8087};
static constexpr uint32_t DEFAULT_COT_STALE{120};
static constexpr double DEFAULT_COT_VAL{9999999.0};
extern const std::string DEFAULT_HOST_ID;
static const char* const DEFAULT_COT_TYPE{"a-u-G"};
static const char* const W3C_XML_DATETIME{"{:%Y-%m-%dT%H:%M:%S}.{:03d}Z"};
static constexpr size_t MAX_UDP_BLOB_SIZE{1400};  // # of bytes
static constexpr size_t MAX_TCP_BLOB_SIZE{64000};  // # of bytes
static constexpr double LATITUDE_BOUND{90.0};  // degrees
static constexpr double LONGITUDE_BOUND{180.0};  // degrees
}  // namespace taktile
