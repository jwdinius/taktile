// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#include "taktile/functions.hpp"

#include <Poco/DOM/AutoPtr.h>
#include <Poco/DOM/DOMWriter.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/URI.h>
#include <Poco/XML/XMLWriter.h>
#include <fmt/chrono.h>
#include <fmt/core.h>

#include <algorithm>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace siomsg = simpleio::messages;

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

void CotType::validate(CotType const& cot) {
  if (cot.lat < -LATITUDE_BOUND || cot.lat > LATITUDE_BOUND) {
    throw std::invalid_argument("Latitude must be between -90 and 90 degrees");
  }
  if (cot.lon < -LONGITUDE_BOUND || cot.lon > LONGITUDE_BOUND) {
    throw std::invalid_argument(
        "Longitude must be between -180 and 180 degrees");
  }
  if (cot.ce < 0) {
    throw std::invalid_argument(
        "Circular Error must be greater than or equal to 0");
  }
  if (cot.hae < 0) {
    throw std::invalid_argument(
        "Height Above Ellipsoid must be greater than or equal to 0");
  }
  if (cot.le < 0) {
    throw std::invalid_argument(
        "Linear Error must be greater than or equal to 0");
  }
  if (cot.uid.empty()) {
    throw std::invalid_argument("UID must not be empty");
  }
  if (cot.cot_type.empty()) {
    throw std::invalid_argument("CoT type must not be empty");
  }
}

CotType::CotType(std::string _uid)
    : uid{std::move(_uid)},
      stale{DEFAULT_COT_STALE},
      cot_type{DEFAULT_COT_TYPE} {}

std::string CotType::get_time(std::optional<int32_t> cot_stale) {
  auto time = std::chrono::system_clock::now();
  if (cot_stale.has_value()) {
    time += std::chrono::seconds(cot_stale.value());
  }
  auto const seconds = std::chrono::time_point_cast<std::chrono::seconds>(time);
  auto const milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>((time - seconds));
  return fmt::format(W3C_XML_DATETIME, seconds, milliseconds.count());
}

siomsg::XmlMessageType Cot2Xml::convert(CotType const& cot) {
  // Create a local Document object
  auto* doc = new Poco::XML::Document();

  // Create <event> element
  auto* event = doc->createElement("event");
  event->setAttribute("version", "2.0");
  event->setAttribute("type", cot.cot_type);
  event->setAttribute("uid", cot.uid);
  event->setAttribute("how", "m-g");
  event->setAttribute("time", CotType::get_time());
  event->setAttribute("start", CotType::get_time());
  event->setAttribute("stale", CotType::get_time(cot.stale));

  // Create <point> element
  auto* point = doc->createElement("point");
  point->setAttribute("lat", std::to_string(cot.lat));
  point->setAttribute("lon", std::to_string(cot.lon));
  point->setAttribute("le", std::to_string(cot.le));
  point->setAttribute("hae", std::to_string(cot.hae));
  point->setAttribute("ce", std::to_string(cot.ce));

  // Create <_flow-tags_> element
  auto* flow_tags = doc->createElement("_flow-tags_");
  std::string _ft_tag = DEFAULT_HOST_ID + "-v" + std::string(VERSION);
  std::replace(_ft_tag.begin(), _ft_tag.end(), '@', '-');
  flow_tags->setAttribute(_ft_tag, CotType::get_time());

  // Create <detail> element
  auto* detail = doc->createElement("detail");
  detail->appendChild(flow_tags);

  event->appendChild(point);
  event->appendChild(detail);

  // Attach <event> to document
  doc->appendChild(event);
  return doc;
}

CotType Cot2Xml::convert(siomsg::XmlMessageType const& xml) {
  // Get the <event> element
  if (xml == nullptr) {
    throw std::invalid_argument("XML message is null.");
  }
  auto* event = xml->documentElement();
  if (event == nullptr || event->nodeName() != "event") {
    throw std::invalid_argument(
        "Expected root-level <event> element not found.");
  }

  // Get the <point> element
  auto* point = event->getChildElement("point");
  if (point == nullptr) {
    throw std::invalid_argument("Expected <point> element not found.");
  }

  // Get the <detail> element
  // auto detail = event->getElementByTagName("detail");
  // if (detail.isNull()) {
  //  throw std::invalid_argument("No <detail> element found");
  //}

  auto uid = event->getAttribute("uid");
  if (uid.empty()) {
    throw std::invalid_argument("UID attribute is empty.");
  }
  auto cot = CotType(uid);

  // Get the attributes
  try {
    cot.lat = std::stod(point->getAttribute("lat"));
    cot.lon = std::stod(point->getAttribute("lon"));
    cot.le = std::stod(point->getAttribute("le"));
    cot.hae = std::stod(point->getAttribute("hae"));
    cot.ce = std::stod(point->getAttribute("ce"));
    cot.cot_type = event->getAttribute("type");
    cot.stale = std::stoul(event->getAttribute("stale"));
    CotType::validate(cot);
    return cot;
  } catch (std::invalid_argument const& e) {
    throw std::invalid_argument("CoT validation failed: " +
                                std::string(e.what()));
  } catch (std::exception const& e) {
    throw std::invalid_argument("Unable to parse: " + std::string(e.what()));
  }
}

CotXmlSerializer::CotXmlSerializer(
    std::shared_ptr<siomsg::XmlSerializer> strategy)
    : xml_serializer_{std::move(strategy)} {}

std::vector<std::byte> CotXmlSerializer::serialize(CotType const& entity) {
  auto xml = Cot2Xml::convert(entity);
  return xml_serializer_->serialize(xml);
}

CotType CotXmlSerializer::deserialize(std::vector<std::byte> const& _blog) {
  auto xml = xml_serializer_->deserialize(_blog);
  return Cot2Xml::convert(xml);
}

CotType hello_event(std::optional<std::string> const& uid) {
  auto cot = CotType(uid.value_or("takPing"));
  cot.cot_type = "t-x-d-d";
  return cot;
}
}  // namespace taktile
