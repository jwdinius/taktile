// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <simpleio/message.hpp>
#include <simpleio/messages/xml.hpp>
#include "taktile/constants.hpp"
#include "taktile/functions.hpp"
/// TODO: put in protobuf subfolder: #include "protobuf/takmessage.pb.h"
#include "takmessage.pb.h"

namespace taktile {

enum class ProtocolVersion : uint8_t {
  V0,  // Cursor-on-Target XML payload: <xml? />
  V1_MESH,  // V1 protobuf with 0xbf <varint 1> 0xbf <PAYLOAD> 
  V1_STREAM // V1 protobuf with 0xbf <varint payload_length> 0xbf <PAYLOAD>
};

/// @brief TakData
class TakData {
 public:
   bool valid();

   TakData();
   TakData(TakData const &) = default;
   TakData(TakData &&) = default;
   TakData &operator=(TakData const &) = default;
   TakData &operator=(TakData &&) = default;
   explicit TakData(std::string const& _uid);
   explicit TakData(atakmap::commoncommo::protobuf::v1::TakMessage proto);
   explicit TakData(simpleio::messages::XmlMessageType xml);
   
   ~TakData() = default;

   atakmap::commoncommo::protobuf::v1::TakMessage proto() const;

   simpleio::messages::XmlMessageType xml() const;

   static TakData hello_event(std::optional<std::string> const &uid);

  private:
   void make_xml();
   atakmap::commoncommo::protobuf::v1::TakMessage proto_;
   simpleio::messages::XmlMessageType xml_;
};

template<size_t N>
class TakDataSerializer : public simpleio::Serializer<TakData, N> 
{
 public:
   TakDataSerializer() = default;
   explicit TakDataSerializer(ProtocolVersion protocol)
   : serialization_protocol_{protocol} {}

   virtual ~TakDataSerializer() = default;

 protected:
   ProtocolVersion const serialization_protocol_{ProtocolVersion::V0};
};

class TakDataSerializerUdp : public TakDataSerializer<MAX_UDP_BLOB_SIZE> {
 public:
   TakDataSerializerUdp() = default;
   explicit TakDataSerializerUdp(ProtocolVersion protocol);

   std::string serialize(TakData const& entity) override;
   TakData deserialize(std::string const& blob) override;

 private:
   simpleio::messages::XmlSerializer<MAX_UDP_BLOB_SIZE> xml_serializer_;   
}; 

class TakDataSerializerTcp : public TakDataSerializer<MAX_TCP_BLOB_SIZE> {
 public:
   TakDataSerializerTcp() = default;
   explicit TakDataSerializerTcp(ProtocolVersion protocol);

   std::string serialize(TakData const& entity) override;
   TakData deserialize(std::string const& blob) override;

 private:
   simpleio::messages::XmlSerializer<MAX_TCP_BLOB_SIZE> xml_serializer_;   
}; 

using TakMessageUdp = simpleio::Message<TakDataSerializerUdp>;
using TakMessageTcp = simpleio::Message<TakDataSerializerTcp>;

}  // namespace taktile
