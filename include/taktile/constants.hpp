#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace taktile {

  enum class Scheme {
    TLS,
    TCP,
    UDP,
    UDP_BROADCAST,
    UDP_WRITE_ONLY,
    LOG
  };

  static const std::unordered_map<Scheme, std::string> SCHEME_FWD_MAP {
    {Scheme::TLS, "tls"},
    {Scheme::TCP, "tcp"},
    {Scheme::UDP, "udp"},
    {Scheme::UDP_BROADCAST, "udp+broadcast"},
    {Scheme::UDP_WRITE_ONLY, "udp+wo"},
    {Scheme::LOG, "log"}
  };

  static const std::unordered_map<std::string, Scheme> SCHEME_INV_MAP {
    {"tls", Scheme::TLS},
    {"tcp", Scheme::TCP},
    {"udp", Scheme::UDP},
    {"udp+broadcast", Scheme::UDP_BROADCAST},
    {"udp+wo", Scheme::UDP_WRITE_ONLY},
    {"log", Scheme::LOG}
  };

  static const std::string DEFAULT_COT_URL {"udp+wo://239.2.3.1:6969"};
  static constexpr uint16_t DEFAULT_BROADCAST_PORT {6969};
  static constexpr uint16_t DEFAULT_COT_PORT {8087};
  static constexpr uint32_t DEFAULT_COT_STALE {120};
  static constexpr double DEFAULT_COT_VAL {9999999.0};
  extern const std::string DEFAULT_HOST_ID;
  static const std::string DEFAULT_COT_TYPE {"a-u-G"};
  static const std::string W3C_XML_DATETIME {"%Y-%m-%dT%H:%M:%S.%fZ"};
} // namespace taktile