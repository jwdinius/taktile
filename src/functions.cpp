#include <algorithm>
#include <stdexcept>

//#include <boost/url.hpp>
#include <Poco/URI.h>

#include "taktile/functions.hpp"

namespace taktile {
URL::URL()
: scheme{Scheme::UDP_WRITE_ONLY}
, net_loc{"239.2.3.1"}
, port{DEFAULT_BROADCAST_PORT} { }

URL::URL(std::string const & url) {
  auto uri = parse_url(url);
  scheme = uri.scheme;
  net_loc = uri.net_loc;
  port = uri.port;
}

URL::URL(Scheme const & _scheme, std::string const & _net_loc, uint16_t _port) {
  scheme = _scheme;
  net_loc = _net_loc;
  port = _port;
}

URL URL::parse_url(std::string const & inp) {
  // Parse the URL
  Poco::URI uri{inp};

  if (SCHEME_INV_MAP.find(uri.getScheme()) == SCHEME_INV_MAP.end()) {
    throw std::invalid_argument("Invalid scheme: " + uri.getScheme());
  }

  auto scheme = SCHEME_INV_MAP.at(uri.getScheme());

  if (uri.getSpecifiedPort() == 0) {
    auto is_broadcast = (uri.getScheme().find("broadcast") != std::string::npos);
    auto is_write_only = (uri.getScheme().find("wo") != std::string::npos);
    return URL{scheme, uri.getHost(), is_broadcast || is_write_only ? DEFAULT_BROADCAST_PORT : DEFAULT_COT_PORT};
  }

  return URL{scheme, uri.getHost(), uri.getSpecifiedPort()};
}
CotMessage::CotMessage(double _lat, double _lon, double _ce, double _hae, double _le,
  std::string _uid, uint32_t _stale, std::string _cot_type) {
  if (_lat < -90.0 || _lat > 90.0) {
    throw std::invalid_argument("Latitude must be between -90 and 90 degrees");
  } else if (_lon < -180 || _lon > 180) {
    throw std::invalid_argument("Longitude must be between -180 and 180 degrees");
  } else if (_ce < 0) {
    throw std::invalid_argument("Circular Error must be greater than or equal to 0");
  } else if (_hae < 0) {
    throw std::invalid_argument("Height Above Ellipsoid must be greater than or equal to 0");
  } else if (_le < 0) {
    throw std::invalid_argument("Linear Error must be greater than or equal to 0");
  } else if (_uid.empty()) {
    throw std::invalid_argument("UID must not be empty");
  } else if (_cot_type.empty()) {
    throw std::invalid_argument("CoT type must not be empty");
  }

  lat = _lat;
  lon = _lon;
  ce = _ce;
  hae = _hae;
  le = _le;
  uid = _uid;
  stale = _stale;
  cot_type = _cot_type;
}

std::string CotMessage::to_xml() const {
  std::string xml = "<event version=\"2.0\" uid=\"" + uid + "\" type=\"" + cot_type + "\" time=\"" + std::to_string(stale) + "\" start=\"" + std::to_string(stale) + "\" stale=\"" + std::to_string(stale) + "\">\n";
  xml += "<point lat=\"" + std::to_string(lat) + "\" lon=\"" + std::to_string(lon) + "\" hae=\"" + std::to_string(hae) + "\" ce=\"" + std::to_string(ce) + "\" le=\"" + std::to_string(le) + "\"/>\n";
  xml += "</event>\n";
  return xml;
}
} // namespace taktile
