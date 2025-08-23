#include <Poco/URI.h>
#include <stdexcept>
#include "taktile/url.hpp"

namespace taktile {
URL::URL(std::string const& url) {
  auto uri = parse_url(url);
  scheme = uri.scheme;
  net_loc = uri.net_loc;
  port = uri.port;
}

URL::URL(Scheme const& _scheme, std::string const& _net_loc, uint16_t _port) {
  scheme = _scheme;
  net_loc = _net_loc;
  port = _port;
}

URL URL::parse_url(std::string const& inp) {
  // Parse the URL
  Poco::URI uri{inp};

  if (SCHEME_INV_MAP.find(uri.getScheme()) == SCHEME_INV_MAP.end()) {
    throw std::invalid_argument("Invalid scheme: " + uri.getScheme());
  }

  auto scheme = SCHEME_INV_MAP.at(uri.getScheme());

  if (uri.getSpecifiedPort() == 0) {
    auto is_broadcast =
        (uri.getScheme().find("broadcast") != std::string::npos);
    auto is_write_only = (uri.getScheme().find("wo") != std::string::npos);
    return URL{scheme, uri.getHost(),
               is_broadcast || is_write_only ? DEFAULT_BROADCAST_PORT
                                             : DEFAULT_COT_PORT};
  }

  return URL{scheme, uri.getHost(), uri.getSpecifiedPort()};
}

} // namespace taktile
