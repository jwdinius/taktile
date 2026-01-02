// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#include "taktile/taktile.hpp"

#include <boost/log/trivial.hpp>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/DOM/DOMWriter.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/XML/XMLWriter.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <simpleio/messages/xml.hpp>

namespace siomsg = simpleio::messages;

namespace taktile {
 
TakDataFramer::TakDataFramer(ProtocolVersion framing_protocol)
  : framing_protocol_{framing_protocol} {}

std::string TakDataFramer::frame(std::string const& entity_blob) const {
  switch (framing_protocol_) {
    case ProtocolVersion::V1_MESH:
      {
        size_t const& header_size = V1_MESH_PROTOCOL_PREFIX.size();
        std::string out(V1_MESH_PROTOCOL_PREFIX.size() + entity_blob.size(), '\0');
        std::memcpy(out.data(), V1_MESH_PROTOCOL_PREFIX.data(), V1_MESH_PROTOCOL_PREFIX.size());
        std::memcpy(out.data() + V1_MESH_PROTOCOL_PREFIX.size(), entity_blob.data(), entity_blob.size());
        return out;
      }
    case ProtocolVersion::V1_STREAM:
      {
        auto const encoded_payload_length = Varint::encode(entity_blob.size());
        size_t const header_size = 1 + encoded_payload_length.size();
        std::string out(header_size + entity_blob.size(), '\0');
        out[0] = V1_PROTOCOL_MAGIC;
        std::memcpy(out.data() + 1, encoded_payload_length.data(), encoded_payload_length.size());
        std::memcpy(out.data() + header_size, entity_blob.data(), entity_blob.size());
        return out;
      }
    case ProtocolVersion::V0:
      {
        std::string out = entity_blob;
        out.insert(0, std::string(V0_PROTOCOL_PREFIX));
        return out;
      }
    default:
      break;
  }
  throw simpleio::SerializerError("Unknown protocol");
}

bool TakDataFramer::try_unframe(std::string& buffer,
                                std::string& entity_blob) const {
  if (buffer.empty()) {
    return false;
  }

  if (buffer.find(V1_MESH_PROTOCOL_PREFIX) != std::string::npos) {
    // Implement deserialization logic for V1_MESH
    auto prefix_pos = buffer.find(V1_MESH_PROTOCOL_PREFIX);
    if (prefix_pos == std::string::npos) {
        // Prefix not found, drop everything
        buffer.clear();
        return false;
    }

    // Drop everything before the prefix
    if (prefix_pos > 0) {
        buffer.erase(0, prefix_pos);
    }

    // Read until the next prefix or end of buffer
    auto end_pos = buffer.find(V1_MESH_PROTOCOL_PREFIX, V1_MESH_PROTOCOL_PREFIX.size());
    if (end_pos == std::string::npos) {
        end_pos = buffer.size();
    }

    TakProto proto;
    if (proto.ParseFromArray(buffer.data() + V1_MESH_PROTOCOL_PREFIX.size(),
                             static_cast<int>(end_pos - V1_MESH_PROTOCOL_PREFIX.size()))) {
      entity_blob = buffer.substr(V1_MESH_PROTOCOL_PREFIX.size(), end_pos);
      buffer.erase(0, end_pos);
      return true;
    }
    return false;
  }
  if (buffer.find(V0_PROTOCOL_PREFIX) != std::string::npos) {
    // Implement deserialization logic for V0
    auto prefix_pos = buffer.find(V0_PROTOCOL_PREFIX);
    if (prefix_pos == std::string::npos) {
        // Prefix not found, drop everything
        buffer.clear();
        return false;
    }

    // Drop everything before the prefix
    if (prefix_pos > 0) {
        buffer.erase(0, prefix_pos);
    }

    // Check if buffer contains a complete CoT message
    if (buffer.find(V0_PROTOCOL_SUFFIX) == std::string::npos) {
      return false;
    }

    // Extract the complete CoT message
    auto end_pos = buffer.find(V0_PROTOCOL_SUFFIX) + V0_PROTOCOL_SUFFIX.size();
    entity_blob = buffer.substr(V0_PROTOCOL_PREFIX.size(), end_pos);
    buffer.erase(0, end_pos);
    return true;
  }
  if (buffer.find(&V1_PROTOCOL_MAGIC) != std::string::npos) {
    auto prefix_pos = buffer.find(&V1_PROTOCOL_MAGIC);
    if (prefix_pos == std::string::npos) {
        // Prefix not found, drop everything
        buffer.clear();
        return false;
    }

    // Drop everything before the prefix
    if (prefix_pos > 0) {
        buffer.erase(0, prefix_pos);
    }

    // 1) Read varint payload length
    taktile::Varint::DecodeResult res{};
    res = Varint::decode(std::string(buffer.data() + 1, buffer.data() + V1_PROTOCOL_MAX_VARINT_SIZE - 1));
    size_t const header_size = 1 + res.bytes_used;

    // 2) Parse protobuf payload
    TakProto proto;
    if (proto.ParseFromArray(buffer.data() + header_size, static_cast<int>(res.payload_length))) {
      entity_blob = buffer.substr(header_size, res.payload_length);
      buffer.erase(0, header_size + res.payload_length);
      return true;
    }
    return false;
  }
  return false;
}

TakData::TakData() {
  auto *cot_event = proto_.mutable_cotevent();
  // POPULATE Tak proto message
  auto const time = TimeProvider::get_time();
  auto const stale_time = TimeProvider::get_time(DEFAULT_COT_STALE);
  cot_event->set_sendtime(time);
  cot_event->set_starttime(time);
  cot_event->set_staletime(stale_time);
  cot_event->set_type(DEFAULT_COT_TYPE);
  cot_event->set_how(DEFAULT_COT_HOW);
  cot_event->set_hae(DEFAULT_COT_VAL);
  cot_event->set_ce(DEFAULT_COT_VAL);
  cot_event->set_le(DEFAULT_COT_VAL);
  make_xml();
}

TakData::TakData(std::string const& _uid) {
  auto *cot_event = proto_.mutable_cotevent();
  auto const time = TimeProvider::get_time();
  auto const stale_time = TimeProvider::get_time(DEFAULT_COT_STALE);
  cot_event->set_uid(_uid);
  cot_event->set_sendtime(time);
  cot_event->set_starttime(time);
  cot_event->set_staletime(stale_time);
  cot_event->set_type(DEFAULT_COT_TYPE);
  cot_event->set_how(DEFAULT_COT_HOW);
  cot_event->set_hae(DEFAULT_COT_VAL);
  cot_event->set_ce(DEFAULT_COT_VAL);
  cot_event->set_le(DEFAULT_COT_VAL);
  make_xml();
}

TakData::TakData(TakProto proto)
  : proto_(std::move(proto)) {
  make_xml();
}

TakData::TakData(simpleio::messages::XmlMessageType xml)
  : xml_(std::move(xml)) {
  assert(xml_ != nullptr && "XML message must not be null");
  auto *root = xml_->documentElement();
  assert(root != nullptr && "Root element must not be null");
  assert(root->nodeName() == "event");
  assert(root->getAttribute("version") == "2.0");
  auto *cot = proto_.mutable_cotevent();
  cot->set_type(root->getAttribute("type"));
  cot->set_uid(root->getAttribute("uid"));
  cot->set_how(root->getAttribute("how"));
  auto const sendtime = TimeProvider::from_datetime(root->getAttribute("time"));
  if (sendtime.has_value()) {
    cot->set_sendtime(sendtime.value());
  }
  auto const starttime = TimeProvider::from_datetime(root->getAttribute("start"));
  if (starttime.has_value()) {
    cot->set_starttime(starttime.value());
  }
  auto const staletime = TimeProvider::from_datetime(root->getAttribute("stale"));
  if (staletime.has_value()) {
    cot->set_staletime(staletime.value());
  }

  auto *point_element = root->getChildElement("point");
  assert(point_element != nullptr && "Point element must not be null");
  cot->set_lat(std::stod(point_element->getAttribute("lat")));
  cot->set_lon(std::stod(point_element->getAttribute("lon")));
  cot->set_le(std::stod(point_element->getAttribute("le")));
  cot->set_hae(std::stod(point_element->getAttribute("hae")));
  cot->set_ce(std::stod(point_element->getAttribute("ce")));

  auto *detail_element = root->getChildElement("detail");
  if (detail_element == nullptr) {
    // parse the detail element and convert to an xml string.
    Poco::XML::DOMWriter writer;
    std::ostringstream oss;
    writer.writeNode(oss, detail_element);
    cot->mutable_detail()->set_xmldetail(oss.str());
  }
}

bool TakData::valid() {
  auto const& event = proto_.cotevent();
  if (event.lat() < -LATITUDE_BOUND || event.lat() > LATITUDE_BOUND) {
    BOOST_LOG_TRIVIAL(error)
        << "Invalid latitude: " << event.lat() << ". Must be between -90 and 90 degrees.";
    return false;
  }
  if (event.lon() < -LONGITUDE_BOUND || event.lon() > LONGITUDE_BOUND) {
    BOOST_LOG_TRIVIAL(error)
        << "Invalid longitude: " << event.lon() << ". Must be between -180 and 180 degrees.";
    return false;
  }
  if (event.ce() < 0) {
    BOOST_LOG_TRIVIAL(error)
        << "Invalid Circular Error: " << event.ce() << ". Must be greater than or equal to 0.";
    return false;
  }
  if (event.hae() < 0) {
    BOOST_LOG_TRIVIAL(error)
        << "Invalid Height Above Ellipsoid: " << event.hae() << ". Must be greater than or equal to 0.";
    return false;
  }
  if (event.le() < 0) {
    BOOST_LOG_TRIVIAL(error)
        << "Invalid Linear Error: " << event.le() << ". Must be greater than or equal to 0.";
    return false;
  }
  if (event.uid().empty()) {
    BOOST_LOG_TRIVIAL(error)
        << "Invalid UID: " << event.uid() << ". Must not be empty.";
    return false;
  }
  if (event.type().empty()) {
    BOOST_LOG_TRIVIAL(error)
        << "Invalid Type: " << event.type() << ". Must not be empty.";
    return false;
  }
  return true;
}

void TakData::make_xml() {
  // Create a local Document object
  xml_ = new Poco::XML::Document();
  auto const& cot = proto_.cotevent();
  // Create <event> element
  auto* event = xml_->createElement("event");
  event->setAttribute("version", "2.0");
  event->setAttribute("type", cot.type());
  event->setAttribute("uid", cot.uid());
  event->setAttribute("how", cot.how());
  event->setAttribute("time", TimeProvider::to_datetime(cot.sendtime()));
  event->setAttribute("start", TimeProvider::to_datetime(cot.starttime()));
  event->setAttribute("stale", TimeProvider::to_datetime(cot.staletime()));

  // Create <point> element
  auto* point = xml_->createElement("point");
  point->setAttribute("lat", std::to_string(cot.lat()));
  point->setAttribute("lon", std::to_string(cot.lon()));
  point->setAttribute("le", std::to_string(cot.le()));
  point->setAttribute("hae", std::to_string(cot.hae()));
  point->setAttribute("ce", std::to_string(cot.ce()));

  // Create <detail> element
  auto* flow_tags = xml_->createElement("_flow-tags_");
  std::string _ft_tag = DEFAULT_HOST_ID + "-v" + std::string(VERSION);
  std::replace(_ft_tag.begin(), _ft_tag.end(), '@', '-');
  flow_tags->setAttribute(_ft_tag, TimeProvider::to_datetime(TimeProvider::get_time()));

  // Create <detail> element
  auto* detail = xml_->createElement("detail");
  /*
  if (!cot.detail().xmldetail().empty()) {
    std::istringstream iss(cot.detail().xmldetail());
    Poco::XML::DOMParser parser;
    Poco::XML::InputSource input_source(iss);
    detail->appendChild(parser.parse(&input_source));
  }
  */
  detail->appendChild(flow_tags);
  event->appendChild(point);
  event->appendChild(detail);

  // Attach <event> to document
  xml_->appendChild(event);
}

TakProto TakData::proto() const {
  return proto_;
}

siomsg::XmlMessageType TakData::xml() const {
  return xml_;
}

TakData TakData::hello_event(std::optional<std::string> const& uid) {
  auto tak_msg = atakmap::commoncommo::protobuf::v1::TakMessage();
  auto *cot = tak_msg.mutable_cotevent();
  cot->set_uid(uid.value_or("takPing"));
  cot->set_type("t-x-d-d");
  return TakData(tak_msg);
}

}  // namespace taktile
