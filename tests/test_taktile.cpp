// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#include <Poco/DOM/AutoPtr.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <gtest/gtest.h>
#include <fmt/core.h>
#include <regex>
#include <string>

#include "taktile/taktile.hpp"

/// Test that creating a CoT message with arbitrary UID returns the expected
/// values.
static inline auto contains_substring =
    [](std::string const& str,
       std::string_view const& sub_str) {
      return std::search(
                 str.begin(),
                 str.end(),
                 sub_str.begin(),
                 sub_str.end()) != str.end();
    };

static inline auto starts_with =
    [](std::string const& str,
       std::string_view const& prefix) {
      return str.size() >= prefix.size() &&
             str.compare(0, prefix.size(), prefix) == 0;
    };

static inline auto ends_with =
    [](std::string const& str,
       std::string_view const& suffix) {
      return str.size() >= suffix.size() &&
             str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    };

namespace sio = simpleio;
namespace siomsg = sio::messages;


TEST(Types, TakData_UidConstructor) {
  auto const uid = "taco";
  auto tak = taktile::TakData(uid);
  EXPECT_TRUE(tak.valid());
  auto serializer = std::make_shared<taktile::TakDataSerializerUdp>();

  // Serialize the message
  auto blob = serializer->serialize(tak);

  // Check that the byte array contains the expected UID ("uid=\"taco\"")
  std::string_view const expected_uid = fmt::format("uid=\"{}\"", uid);
  EXPECT_TRUE(contains_substring(blob, expected_uid));

  // Check that the byte array contains the expected type ("type=\"a-u-G\"")
  std::string_view const expected_cot_type =
      fmt::format("type=\"{}\"", taktile::DEFAULT_COT_TYPE);
  EXPECT_TRUE(contains_substring(blob, expected_cot_type));
}

TEST(Types, TakData_HelloEvent) {
  auto const uid = "taco";
  auto tak = taktile::TakData::hello_event(uid);
  EXPECT_TRUE(tak.valid());
  auto serializer = std::make_shared<taktile::TakDataSerializerTcp>();

  // Serialize the message
  auto blob = serializer->serialize(tak);

  // Check that the byte array contains the expected UID ("uid=\"taco\"")
  std::string_view const expected_uid = fmt::format("uid=\"{}\"", uid);
  EXPECT_TRUE(contains_substring(blob, expected_uid));

  // Check that the byte array contains the expected type ("type=\"t-x-d-d\"")
  std::string_view const expected_cot_type =
      fmt::format("type=\"{}\"", "t-x-d-d");
  EXPECT_TRUE(contains_substring(blob, expected_cot_type));
}

TEST(Types, TakData_XmlFromProto) {
  // Test that the CoT message is correctly converted to an XML document string.
  atakmap::commoncommo::protobuf::v1::TakMessage proto;
  auto cot = proto.mutable_cotevent();
  cot->set_uid("test_uid");
  cot->set_how(taktile::DEFAULT_COT_HOW);
  cot->set_lat(37.7749);
  cot->set_lon(-122.4194);
  cot->set_le(10);
  cot->set_hae(100);
  cot->set_ce(5);
  cot->set_type("a-f-G");
  auto time = taktile::TimeProvider::get_time();
  auto staletime = taktile::TimeProvider::get_time(3600000);
  cot->set_sendtime(time);
  cot->set_starttime(time);
  cot->set_staletime(staletime);
  auto tak = taktile::TakData(proto);
  EXPECT_TRUE(tak.valid());

  auto doc = tak.xml();
  EXPECT_NE(doc, nullptr);
  auto root = doc->documentElement();
  EXPECT_NE(root, nullptr);
  EXPECT_EQ(root->nodeName(), "event");
  EXPECT_EQ(root->getAttribute("version"), "2.0");
  EXPECT_EQ(root->getAttribute("type"), cot->type());
  EXPECT_EQ(root->getAttribute("uid"), cot->uid());
  EXPECT_EQ(root->getAttribute("how"), taktile::DEFAULT_COT_HOW);
  EXPECT_TRUE(
      std::regex_match(root->getAttribute("time"), taktile::W3C_XML_DATETIME_REGEX));
  EXPECT_TRUE(
      std::regex_match(root->getAttribute("start"), taktile::W3C_XML_DATETIME_REGEX));
  EXPECT_TRUE(
      std::regex_match(root->getAttribute("stale"), taktile::W3C_XML_DATETIME_REGEX));

  auto point_element = root->getChildElement("point");
  EXPECT_NE(point_element, nullptr);
  EXPECT_EQ(point_element->getAttribute("lat"), fmt::format("{:.6f}", cot->lat()));
  EXPECT_EQ(point_element->getAttribute("lon"), fmt::format("{:.6f}", cot->lon()));
  EXPECT_EQ(point_element->getAttribute("le"), fmt::format("{:.6f}", cot->le()));
  EXPECT_EQ(point_element->getAttribute("hae"), fmt::format("{:.6f}", cot->hae()));
  EXPECT_EQ(point_element->getAttribute("ce"), fmt::format("{:.6f}", cot->ce()));

  auto detail_element = root->getChildElement("detail");
  EXPECT_NE(detail_element, nullptr);
  auto flow_tags_element = detail_element->getChildElement("_flow-tags_");
  EXPECT_NE(flow_tags_element, nullptr);
  auto const _ft_tag =
      fmt::format("{}-v{}", taktile::DEFAULT_HOST_ID, taktile::VERSION);
  auto const _ft_tag_replaced =
      std::regex_replace(_ft_tag, std::regex("@"), "-");
  // Check that the flow tags element contains the expected tag
  EXPECT_NE(flow_tags_element->getAttribute(_ft_tag_replaced), "");
}

// Example test case using the test harness
TEST(Types, TakData_ProtoFromHelloEvent) {
  auto hello_event = taktile::TakData::hello_event("taco");

  auto tak_msg =
      std::make_shared<sio::Message<taktile::TakDataSerializerUdp>>(std::move(hello_event));
  {
    auto entity = tak_msg->entity();
    EXPECT_EQ(entity.proto().cotevent().uid(), "taco");
  }

  std::string serialized_tak_msg{tak_msg->blob()};

  auto tak_msg_from_serialized =
      std::make_shared<sio::Message<taktile::TakDataSerializerUdp>>(std::move(serialized_tak_msg));

  {
    auto entity = tak_msg_from_serialized->entity();
    EXPECT_EQ(entity.proto().cotevent().uid(), "taco");
  }
}

TEST(Types, TakMessage_V0_SerializeAndDeserialize) {
  // Create a TakData object (using the hello_event function)
  auto hello_event = taktile::TakData::hello_event("taco");
  auto serializer = std::make_shared<taktile::TakDataSerializerUdp>(true);
  auto tak_msg =
      std::make_shared<taktile::TakMessageUdp>(std::move(hello_event), serializer);
  {
    auto entity = tak_msg->entity();
    EXPECT_EQ(entity.proto().cotevent().uid(), "taco");
  }

  // Copy the packed entity
  std::string serialized_tak_msg{tak_msg->blob()};

  auto tak_msg_from_serialized =
      std::make_shared<taktile::TakMessageUdp>(std::move(serialized_tak_msg), serializer);

  {
    auto entity = tak_msg_from_serialized->entity();
    EXPECT_EQ(entity.proto().cotevent().uid(), "taco");
  }
}

TEST(Types, TakMessage_V0_FrameAndUnframe) {
  // Create a TakData object (using the hello_event function)
  auto hello_event = taktile::TakData::hello_event("taco");
  auto serializer = std::make_shared<taktile::TakDataSerializerUdp>(true);
  auto tak_msg =
      std::make_shared<taktile::TakMessageUdp>(std::move(hello_event), serializer);
  {
    auto entity = tak_msg->entity();
    EXPECT_EQ(entity.proto().cotevent().uid(), "taco");
  }
  // Copy the packed entity
  std::string serialized_tak_msg{tak_msg->blob()};

  // Frame the message
  auto framer = std::make_shared<taktile::TakDataFramer>(taktile::ProtocolVersion::V0);
  auto framed_msg = framer->frame(serialized_tak_msg);

  // Check that the framed message contains the expected XML prefix and suffix
  EXPECT_TRUE(starts_with(framed_msg, std::string(taktile::V0_PROTOCOL_PREFIX)));

  // Check that the framed message contains the expected XML suffix
  EXPECT_TRUE(ends_with(framed_msg, std::string(taktile::V0_PROTOCOL_SUFFIX)));

  // Unframe the message
  std::string buffer = framed_msg;
  std::string entity_blob;
  EXPECT_TRUE(framer->try_unframe(buffer, entity_blob));
  EXPECT_EQ(buffer.size(), 0);
  EXPECT_EQ(entity_blob, serialized_tak_msg);
}

TEST(Types, TakMessage_V1_Mesh_SerializeAndDeserialize) {
  // Create a TakData object (using the hello_event function)
  auto hello_event = taktile::TakData::hello_event("taco");
  auto serializer = std::make_shared<taktile::TakDataSerializerUdp>(false);
  auto tak_msg =
      std::make_shared<sio::Message<taktile::TakDataSerializerUdp>>(std::move(hello_event), serializer);
  {
    auto entity = tak_msg->entity();
    EXPECT_EQ(entity.proto().cotevent().uid(), "taco");
  }

  // Copy the packed entity
  std::string serialized_tak_msg{tak_msg->blob()};

  auto tak_msg_from_serialized =
      std::make_shared<sio::Message<taktile::TakDataSerializerUdp>>(std::move(serialized_tak_msg), serializer);

  {
    auto entity = tak_msg_from_serialized->entity();
    EXPECT_EQ(entity.proto().cotevent().uid(), "taco");
  }
}

TEST(Types, TakMessage_V1_MESH_FrameAndUnframe) {
  // Create a TakData object (using the hello_event function)
  auto hello_event = taktile::TakData::hello_event("taco");
  auto serializer = std::make_shared<taktile::TakDataSerializerUdp>(false);
  auto tak_msg =
      std::make_shared<taktile::TakMessageUdp>(std::move(hello_event), serializer);
  {
    auto entity = tak_msg->entity();
    EXPECT_EQ(entity.proto().cotevent().uid(), "taco");
  }

  // Copy the packed entity
  std::string serialized_tak_msg{tak_msg->blob()};

  // Frame the message
  auto framer = std::make_shared<taktile::TakDataFramer>(taktile::ProtocolVersion::V1_MESH);
  auto framed_msg = framer->frame(serialized_tak_msg);

  // Check that the framed message contains the expected V1 mesh prefix and suffix
  EXPECT_TRUE(starts_with(framed_msg, std::string(taktile::V1_MESH_PROTOCOL_PREFIX)));

  // Unframe the message
  std::string buffer = framed_msg;
  std::string entity_blob;
  EXPECT_TRUE(framer->try_unframe(buffer, entity_blob));
  EXPECT_EQ(buffer.size(), 0);
  EXPECT_EQ(entity_blob, serialized_tak_msg);
}

TEST(Types, TakMessage_V1_STREAM_SerializeAndDeserialize) {
  // Create a TakData object (using the hello_event function)
  auto hello_event = taktile::TakData::hello_event("taco");
  auto serializer = std::make_shared<taktile::TakDataSerializerTcp>(false);
  auto tak_msg =
      std::make_shared<sio::Message<taktile::TakDataSerializerTcp>>(std::move(hello_event), serializer);
  {
    auto entity = tak_msg->entity();
    EXPECT_EQ(entity.proto().cotevent().uid(), "taco");
  }

  // Copy the packed entity
  std::string serialized_tak_msg{tak_msg->blob()};

  auto tak_msg_from_serialized =
      std::make_shared<sio::Message<taktile::TakDataSerializerTcp>>(std::move(serialized_tak_msg), serializer);

  {
    auto entity = tak_msg_from_serialized->entity();
    EXPECT_EQ(entity.proto().cotevent().uid(), "taco");
  }
}

TEST(Types, TakMessage_V1_STREAM_FrameAndUnframe) {
  // Create a TakData object (using the hello_event function)
  auto hello_event = taktile::TakData::hello_event("taco");
  auto serializer = std::make_shared<taktile::TakDataSerializerUdp>(false);
  auto tak_msg =
      std::make_shared<taktile::TakMessageUdp>(std::move(hello_event), serializer);
  {
    auto entity = tak_msg->entity();
    EXPECT_EQ(entity.proto().cotevent().uid(), "taco");
  }

  // Copy the packed entity
  std::string serialized_tak_msg{tak_msg->blob()};

  // Frame the message
  auto framer = std::make_shared<taktile::TakDataFramer>(taktile::ProtocolVersion::V1_STREAM);
  auto framed_msg = framer->frame(serialized_tak_msg);

  // Check that the framed message contains the expected V1 mesh prefix and suffix
  EXPECT_TRUE(starts_with(framed_msg, std::string(&taktile::V1_PROTOCOL_MAGIC)));

  // Unframe the message
  std::string buffer = framed_msg;
  std::string entity_blob;
  EXPECT_TRUE(framer->try_unframe(buffer, entity_blob));
  EXPECT_EQ(buffer.size(), 0);
  EXPECT_EQ(entity_blob, serialized_tak_msg);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}