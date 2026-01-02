// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "simpleio/message.hpp"
#include "simpleio/messages/xml.hpp"
#include "taktile/constants.hpp"
#include "taktile/functions.hpp"
#include "takproto/takmessage.pb.h"

namespace taktile {

using TakProto = atakmap::commoncommo::protobuf::v1::TakMessage;

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
   explicit TakData(TakProto proto);
   explicit TakData(simpleio::messages::XmlMessageType xml);
   
   ~TakData() = default;

   TakProto proto() const;

   simpleio::messages::XmlMessageType xml() const;

   static TakData hello_event(std::optional<std::string> const &uid);

  private:
   void make_xml();
   TakProto proto_;
   simpleio::messages::XmlMessageType xml_;
};

template<size_t N>
class TakProtoSerializer : public simpleio::Serializer<TakProto, N> 
{
 public:
   TakProtoSerializer() = default;

   std::string serialize(TakProto const& entity) override {
     // Serialize protobuf payload
     std::string payload;
     if (!entity.SerializeToString(&payload)) {
       throw simpleio::SerializerError("Failed to serialize TakMessage to string.");
     }
     if (payload.size() > N) {
       throw simpleio::SerializerError("Payload too large for serializer");
     }
     return payload;
   }

   TakProto deserialize(std::string const& blob) override {
     TakProto proto;
     if (!proto.ParseFromArray(blob.data(), static_cast<int>(blob.size()))) {
       throw simpleio::SerializerError("Failed to deserialize TakMessage from string.");
     }
     return proto;
   }
};

template<size_t N>
class TakDataSerializer : public simpleio::Serializer<TakData, N> 
{
 public:
   TakDataSerializer() = default;
   explicit TakDataSerializer(bool use_v0_protocol)
       : use_v0_protocol_{use_v0_protocol} {}

   virtual ~TakDataSerializer() = default;

   std::string serialize(TakData const& entity) override {
      if (use_v0_protocol_) {
          return xml_serializer_.serialize(entity.xml());
      }
      return proto_serializer_.serialize(entity.proto());
   }

    TakData deserialize(std::string const& blob) override {
      try {
        auto proto = proto_serializer_.deserialize(blob);
        return TakData(proto);
      } catch (simpleio::SerializerError&) {
        // Fall through to XML deserialization 
        return TakData(xml_serializer_.deserialize(blob));
      }
    }

 private:
   bool const use_v0_protocol_{true};
   TakProtoSerializer<N> proto_serializer_;
   simpleio::messages::XmlSerializer<N> xml_serializer_;   
};

class TakDataFramer : public simpleio::Framer {
 public:
   TakDataFramer() = default;
   explicit TakDataFramer(ProtocolVersion framing_protocol);

    [[nodiscard]] std::string frame(
        std::string const& entity_blob) const override;

    bool try_unframe(std::string& buffer,
                     std::string& entity_blob) const override;

 private:
   ProtocolVersion const framing_protocol_{ProtocolVersion::V0};
};

using TakDataSerializerUdp = TakDataSerializer<MAX_UDP_BLOB_SIZE>;
using TakDataSerializerTcp = TakDataSerializer<MAX_TCP_BLOB_SIZE>;
using TakMessageUdp = simpleio::Message<TakDataSerializerUdp>;
using TakMessageTcp = simpleio::Message<TakDataSerializerTcp>;

}  // namespace taktile
