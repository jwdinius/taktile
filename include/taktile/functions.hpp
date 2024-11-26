#pragma once

#include <string>
#include <cstdint>

namespace taktile {
  static constexpr uint16_t DEFAULT_BROADCAST_PORT {8087};

  struct url {
    std::string scheme;
    std::string netloc;
    uint16_t port {DEFAULT_BROADCAST_PORT};
  };

  url parse_url(std::string const & inp);
} // namespace taktile