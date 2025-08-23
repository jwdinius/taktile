#pragma once

#include <string>

#include "taktile/constants.hpp"

namespace taktile {
/// @brief A URL structure - see RFC-1808
///        https://datatracker.ietf.org/doc/html/rfc1808.html
/// @note The default constructor initializes the URL object as DEFAULT_COT_URL.
class URL {
 public:
  Scheme scheme{Scheme::UDP_WRITE_ONLY};
  std::string net_loc{DEFAULT_IPV4_ADDRESS};
  uint16_t port{DEFAULT_BROADCAST_PORT};

  URL() = default;
  ~URL() = default;
  URL(URL const &) = default;
  URL(URL &&) = default;
  URL &operator=(URL const &) = default;
  URL &operator=(URL &&) = default;

  /// @brief Construct a URL object with the given URL string
  /// @param url
  /// @throws std::invalid_argument if the scheme is not in SCHEME
  explicit URL(std::string const &url);

  /// @brief Construct a URL object with the given scheme, net_loc, and port
  /// @param _scheme
  /// @param _net_loc
  /// @param _port
  URL(Scheme const &_scheme, std::string const &_net_loc, uint16_t _port);

 private:
  /// @brief Parse a string into a URL structure
  /// @param inp
  /// @return url
  /// @throws std::invalid_argument if the scheme is not in SCHEME
  static URL parse_url(std::string const &inp);
};
} // namespace taktile