#include <algorithm>
#include <chrono>
#include <optional>
#include <sstream>
#include <stdexcept>

//#include <boost/url.hpp>
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/DOMWriter.h>
#include <Poco/DOM/Element.h>
#include <Poco/URI.h>
#include <Poco/XML/XMLWriter.h>

#include "taktile/functions.hpp"

namespace taktile {

using namespace std::chrono_literals;

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

CotMessage::CotMessage(std::string _uid)
: CotMessage{0.0, 0.0, 0.0, 0.0, 0.0, _uid, DEFAULT_COT_STALE, DEFAULT_COT_TYPE} { }

std::string CotMessage::as_xml_string() const {
  // Create a local Document object
  Poco::XML::AutoPtr<Poco::XML::Document> doc = new Poco::XML::Document();

  // Create <event> element
  Poco::XML::Element* event = doc->createElement("event");
  event->setAttribute("version", "2.0");
  event->setAttribute("type", cot_type);
  event->setAttribute("uid", uid);
  event->setAttribute("how", "m-g");
  event->setAttribute("time", get_time());
  event->setAttribute("start", get_time());
  event->setAttribute("stale", get_time(stale));

  // Create <point> element
  Poco::XML::Element* point = doc->createElement("point");
  point->setAttribute("lat", std::to_string(lat));
  point->setAttribute("lon", std::to_string(lon));
  point->setAttribute("le", std::to_string(le));
  point->setAttribute("hae", std::to_string(hae));
  point->setAttribute("ce", std::to_string(ce));

  // Create <_flow-tags_> element
  Poco::XML::Element* flow_tags = doc->createElement("_flow-tags_");
  std::string _ft_tag = DEFAULT_HOST_ID + "-v" + std::string(VERSION);
  std::replace(_ft_tag.begin(), _ft_tag.end(), '@', '-');
  flow_tags->setAttribute(_ft_tag, get_time());
  
  // Create <detail> element
  Poco::XML::Element* detail = doc->createElement("detail");
  detail->appendChild(flow_tags);

  event->appendChild(point);
  event->appendChild(detail);

  // Attach <event> to document
  doc->appendChild(event);

  std::ostringstream oss;
  Poco::XML::DOMWriter writer;
  writer.setOptions(Poco::XML::XMLWriter::PRETTY_PRINT);
  writer.writeNode(oss, doc);
  return oss.str();
}

std::vector<uint8_t> CotMessage::as_byte_array() const {
  auto xml = this->as_xml_string();
  return std::vector<uint8_t>(xml.begin(), xml.end());
}

std::string CotMessage::get_time(std::optional<int32_t> cot_stale) {
  auto time = std::chrono::system_clock::now();
  if (cot_stale.has_value()) {
    time += std::chrono::seconds(cot_stale.value());
  }
  auto const seconds = std::chrono::time_point_cast<std::chrono::seconds>(time);
  auto const milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>((time - seconds));
  return fmt::format(W3C_XML_DATETIME, seconds, milliseconds.count());
}

CotMessage hello_event(std::optional<std::string> uid) {
  return CotMessage{0.0, 0.0, 0.0, 0.0, 0.0, uid.value_or("takPing"), 0, "t-x-d-d"};
}
} // namespace taktile
