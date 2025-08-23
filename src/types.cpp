// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#include "taktile/constants.hpp"
#include "taktile/functions.hpp"
#include "taktile/types.hpp"

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
 
std::string serialize_v1_mesh(TakData const &entity, size_t max_blob_size) {
  auto const& proto = entity.proto();
  // 1) Serialize protobuf payload
  std::string payload;
  if (!proto.SerializeToString(&payload)) {
    throw simpleio::SerializerError("Failed to serialize TakMessage to string.");
  }

  // 2) Prepend 0xBF 0x01 0xBF
  std::string out;
  out.reserve(V1_MESH_PROTOCOL_PREFIX.size() + payload.size());
  out.append(reinterpret_cast<const char*>(V1_MESH_PROTOCOL_PREFIX.data()),
             V1_MESH_PROTOCOL_PREFIX.size());
  out += payload;
  if (out.size() > max_blob_size) {
    throw simpleio::SerializerError("Payload too large for mesh frame");
  }
  return out;
}
   
TakData deserialize_v1_mesh(std::string const &blob) {
  // Implement deserialization logic for V1_MESH
   atakmap::commoncommo::protobuf::v1::TakMessage proto;
   if (!proto.ParseFromArray(blob.data() + V1_MESH_PROTOCOL_PREFIX.size(),
                              static_cast<int>(blob.size() - V1_MESH_PROTOCOL_PREFIX.size()))) {
     throw simpleio::SerializerError("Failed to deserialize TakMessage from string.");
   }
  return TakData(proto);
}

std::string serialize_v1_stream(TakData const &entity, size_t max_blob_size) {
   // 1) Serialize protobuf payload
   auto const& proto = entity.proto();
   std::string payload;
   if (!proto.SerializeToString(&payload)) {
     throw simpleio::SerializerError("Failed to serialize TakMessage to string.");
   }

   auto encoded_payload_length = Varint::encode(payload.size());

   size_t const header = V1_PROTOCOL_MAGIC_SIZE + encoded_payload_length.size();
   // (optional) enforce a max blob size for safety
   if (payload.size() > max_blob_size - header) {
     throw simpleio::SerializerError("Payload too large for stream frame");
   }

   // 2) Build frame: 0xBF <varint length> <payload>
   std::string out(header + payload.size(), '\0');
   out[0] = V1_PROTOCOL_MAGIC;
   std::memcpy(out.data() + V1_PROTOCOL_MAGIC_SIZE, encoded_payload_length.data(), encoded_payload_length.size());
   std::memcpy(out.data() + header, payload.data(), payload.size());

   return out;
}

TakData deserialize_v1_stream(std::string const &blob, size_t max_blob_size) {
   // 1) Validate magic
   if (blob.size() < 2 || static_cast<unsigned char>(blob[0]) != V1_PROTOCOL_MAGIC) {
     throw simpleio::SerializerError("Invalid TAK v1 stream header");
   }

   // 2) Read varint payload length
   taktile::Varint::DecodeResult res{};
   try {
     res = Varint::decode(std::string(blob.data() + 1, blob.size() - 1));
   } catch (std::exception const& e) {
     throw simpleio::SerializerError(std::string("Failed to read stream length: ") + e.what());
   }

   size_t const header = V1_PROTOCOL_MAGIC_SIZE + res.bytes_used;
   if (header + res.payload_length > blob.size()) {
     throw simpleio::SerializerError("Incomplete TAK v1 stream frame");
   }
   if (res.payload_length > max_blob_size - header) {
     throw simpleio::SerializerError("Stream payload exceeds maximum size");
   }

   // 3) Parse protobuf payload
   atakmap::commoncommo::protobuf::v1::TakMessage proto;
   if (!proto.ParseFromArray(blob.data() + header, static_cast<int>(res.payload_length))) {
     throw simpleio::SerializerError("Failed to parse TakMessage from stream frame");
   }

   return TakData(proto);
}

ProtocolVersion extract_protocol(std::string const &blob) {
   // 1) Check v1 MESH: 0xBF 0x01 0xBF
   if (blob.size() >= V1_MESH_PROTOCOL_PREFIX.size() &&
       std::memcmp(blob.data(),
                   V1_MESH_PROTOCOL_PREFIX.data(),
                   V1_MESH_PROTOCOL_PREFIX.size()) == 0) {
     return ProtocolVersion::V1_MESH;
   }

   // 2) Check v1 STREAM: leading 0xBF (but not matching mesh above)
   if (!blob.empty() && static_cast<char>(blob.front()) == V1_PROTOCOL_MAGIC) {
     return ProtocolVersion::V1_STREAM;
   }

   // 3) Check if V0 XML.
   if (blob.rfind(V0_PROTOCOL_PREFIXES[0], 0) == 0 || blob.rfind(V0_PROTOCOL_PREFIXES[1], 0) == 0) {
     return ProtocolVersion::V0;
   }

   throw simpleio::SerializerError("Unknown protocol");
}

TakDataSerializerUdp::TakDataSerializerUdp(ProtocolVersion protocol)
  : TakDataSerializer<MAX_UDP_BLOB_SIZE>(protocol) {}

std::string TakDataSerializerUdp::serialize(TakData const& entity) {
  switch(this->serialization_protocol_) {
    case ProtocolVersion::V1_MESH:
      return serialize_v1_mesh(entity, MAX_UDP_BLOB_SIZE);
    case ProtocolVersion::V1_STREAM:
      return serialize_v1_stream(entity, MAX_UDP_BLOB_SIZE);
    case ProtocolVersion::V0:
      return xml_serializer_.serialize(entity.xml());
  }
  throw simpleio::SerializerError("Unknown protocol");
}

TakData TakDataSerializerUdp::deserialize(std::string const& blob) {
  switch (extract_protocol(blob)) {
    case ProtocolVersion::V1_MESH:
      return deserialize_v1_mesh(blob);
    case ProtocolVersion::V1_STREAM:
      return deserialize_v1_stream(blob, MAX_UDP_BLOB_SIZE);
    case ProtocolVersion::V0:
      return TakData(xml_serializer_.deserialize(blob));
  }
  throw simpleio::SerializerError("Unknown protocol");
}

TakDataSerializerTcp::TakDataSerializerTcp(ProtocolVersion protocol)
  : TakDataSerializer<MAX_TCP_BLOB_SIZE>(protocol) {}

std::string TakDataSerializerTcp::serialize(TakData const& entity) {
  switch(this->serialization_protocol_) {
    case ProtocolVersion::V1_MESH:
      return serialize_v1_mesh(entity, MAX_TCP_BLOB_SIZE);
    case ProtocolVersion::V1_STREAM:
      return serialize_v1_stream(entity, MAX_TCP_BLOB_SIZE);
    case ProtocolVersion::V0:
      return xml_serializer_.serialize(entity.xml());
  }
  throw simpleio::SerializerError("Unknown protocol");
}

TakData TakDataSerializerTcp::deserialize(std::string const& blob) {
  switch (extract_protocol(blob)) {
    case ProtocolVersion::V1_MESH:
      return deserialize_v1_mesh(blob);
    case ProtocolVersion::V1_STREAM:
      return deserialize_v1_stream(blob, MAX_TCP_BLOB_SIZE);
    case ProtocolVersion::V0:
      return TakData(xml_serializer_.deserialize(blob));
  }
  throw simpleio::SerializerError("Unknown protocol");
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

TakData::TakData(atakmap::commoncommo::protobuf::v1::TakMessage proto)
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

atakmap::commoncommo::protobuf::v1::TakMessage TakData::proto() const {
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
