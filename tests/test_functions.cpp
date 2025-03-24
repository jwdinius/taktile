// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#include <Poco/DOM/AutoPtr.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <fmt/core.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <regex>

#include <simpleio/messages/xml.hpp>
#include "taktile/functions.hpp"

static std::regex const W3C_XML_DATETIME_REGEX(
    R"(^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d{3}Z$)");

/// Test that creating a cot message with arbitrary UID returns the expected
/// values.
static inline auto contains_substring([](std::vector<uint8_t> const& byte_array,
                                         std::string_view const& sub_str) {
  return std::search(byte_array.begin(), byte_array.end(), sub_str.begin(),
                     sub_str.end()) != byte_array.end();
});

TEST(Functions, test_construct_cot_url_default) {
  /// Test case that checks default-constructed URL is the same as the default
  /// CoT URL
  auto result = taktile::URL();
  auto expected = taktile::URL(taktile::Scheme::UDP_WRITE_ONLY,
                               taktile::DEFAULT_IPV4_ADDRESS,
                               taktile::DEFAULT_BROADCAST_PORT);
  EXPECT_EQ(result.scheme, expected.scheme);
  EXPECT_EQ(result.net_loc, expected.net_loc);
  EXPECT_EQ(result.port, expected.port);
}

TEST(Functions, construct_cot_url_https_no_port) {
  /// Test case for constructing a URL with the HTTPS scheme and no port
  auto result = taktile::URL("https://www.example.com");
  auto expected = taktile::URL{taktile::Scheme::HTTPS, "www.example.com",
                               taktile::DEFAULT_COT_PORT};
  EXPECT_EQ(result.scheme, expected.scheme);
  EXPECT_EQ(result.net_loc, expected.net_loc);
  EXPECT_EQ(result.port, expected.port);
}

TEST(Functions, construct_cot_url_tcp_no_port) {
  /// Test case for constructing a URL with the TCP scheme and no port
  auto result = taktile::URL("tcp://www.example.com");
  auto expected = taktile::URL{taktile::Scheme::TCP, "www.example.com",
                               taktile::DEFAULT_COT_PORT};
  EXPECT_EQ(result.scheme, expected.scheme);
  EXPECT_EQ(result.net_loc, expected.net_loc);
  EXPECT_EQ(result.port, expected.port);
}

TEST(Functions, construct_cot_url_tls_no_port) {
  /// Test case for constructing a URL with the TLS scheme and no port
  auto result = taktile::URL("tls://www.example.com");
  auto expected = taktile::URL{taktile::Scheme::TLS, "www.example.com",
                               taktile::DEFAULT_COT_PORT};
  EXPECT_EQ(result.scheme, expected.scheme);
  EXPECT_EQ(result.net_loc, expected.net_loc);
  EXPECT_EQ(result.port, expected.port);
}

TEST(Functions, construct_cot_url_udp_port) {
  /// Test case for constructing a URL with the UDP scheme and a port
  auto result = taktile::URL("udp://www.example.com:9999");
  auto expected = taktile::URL{taktile::Scheme::UDP, "www.example.com", 9999};
  EXPECT_EQ(result.scheme, expected.scheme);
  EXPECT_EQ(result.net_loc, expected.net_loc);
  EXPECT_EQ(result.port, expected.port);
}

TEST(Functions, construct_cot_url_udp_broadcast_no_port) {
  /// Test case for constructing a URL with the UDP+BROADCAST scheme and no port
  auto result = taktile::URL("udp+broadcast://www.example.com");
  auto expected =
      taktile::URL{taktile::Scheme::UDP_BROADCAST, "www.example.com",
                   taktile::DEFAULT_BROADCAST_PORT};
  EXPECT_EQ(result.scheme, expected.scheme);
  EXPECT_EQ(result.net_loc, expected.net_loc);
  EXPECT_EQ(result.port, expected.port);
}

TEST(Functions, construct_cot_url_invalid_scheme_throws) {
  /// Test case for constructing a URL with invalid scheme.
  EXPECT_THROW(taktile::URL("www.example.com"), std::invalid_argument);
}

TEST(Functions, cot_message_get_time) {
  /// Test that get_time returns the current system time in W3C XML datetime
  /// format
  auto result = taktile::CotType::get_time();
  EXPECT_TRUE(std::regex_match(result, W3C_XML_DATETIME_REGEX));
}

TEST(Functions, cot_message_byte_array) {
  auto const uid = "taco";
  auto cot = taktile::CotType(uid);
  auto strategy = std::make_shared<simpleio::messages::XmlSerializer>();
  auto cot_msg = taktile::CotMessage<taktile::MAX_UDP_BLOB_SIZE>(
      std::move(cot), strategy);

  // Check that the byte array contains the expected UID ("uid=\"taco\"")
  std::string_view const expected_uid = fmt::format("uid=\"{}\"", uid);
  EXPECT_TRUE(contains_substring(cot_msg.blob(), expected_uid));

  // Check that the byte array contains the expected type ("type=\"a-u-G\"")
  std::string_view const expected_cot_type =
      fmt::format("type=\"{}\"", taktile::DEFAULT_COT_TYPE);
  EXPECT_TRUE(contains_substring(cot_msg.blob(), expected_cot_type));
}

TEST(Functions, hello_event) {
  auto const uid = "taco";
  auto cot = taktile::hello_event(uid);
  auto strategy = std::make_shared<simpleio::messages::XmlSerializer>();
  auto cot_msg = taktile::CotMessage<taktile::MAX_UDP_BLOB_SIZE>(
      std::move(cot), strategy);

  // Check that the byte array contains the expected UID ("uid=\"taco\"")
  std::string_view const expected_uid = fmt::format("uid=\"{}\"", uid);
  EXPECT_TRUE(contains_substring(cot_msg.blob(), expected_uid));

  // Check that the byte array contains the expected type ("type=\"t-x-d-d\"")
  std::string_view const expected_cot_type =
      fmt::format("type=\"{}\"", "t-x-d-d");
  EXPECT_TRUE(contains_substring(cot_msg.blob(), expected_cot_type));
}

TEST(Functions, cot2xml) {
  // Test that the CoT message is correctly converted to an XML document string.
  auto const cot = taktile::CotType(37.7749, -122.4194, 10, 100, 5,
                                           "test_uid", 3600, "a-f-G");
  auto strategy = std::make_shared<simpleio::messages::XmlSerializer>();
  auto cot_msg = taktile::CotMessage<taktile::MAX_UDP_BLOB_SIZE>(
      std::move(cot), strategy);
  auto doc = cot_msg.entity();
  EXPECT_NE(doc, nullptr);
  auto root = doc->documentElement();
  EXPECT_NE(root, nullptr);
  EXPECT_EQ(root->nodeName(), "event");
  EXPECT_EQ(root->getAttribute("version"), "2.0");
  EXPECT_EQ(root->getAttribute("type"), cot_msg.cot_type);
  EXPECT_EQ(root->getAttribute("uid"), cot_msg.uid);
  EXPECT_EQ(root->getAttribute("how"), "m-g");
  EXPECT_TRUE(
      std::regex_match(root->getAttribute("time"), W3C_XML_DATETIME_REGEX));
  EXPECT_TRUE(
      std::regex_match(root->getAttribute("start"), W3C_XML_DATETIME_REGEX));
  EXPECT_TRUE(
      std::regex_match(root->getAttribute("stale"), W3C_XML_DATETIME_REGEX));

  auto point_element = root->getChildElement("point");
  EXPECT_NE(point_element, nullptr);
  EXPECT_EQ(point_element->getAttribute("lat"),
            fmt::format("{:.6f}", cot_msg.lat));
  EXPECT_EQ(point_element->getAttribute("lon"),
            fmt::format("{:.6f}", cot_msg.lon));
  EXPECT_EQ(point_element->getAttribute("le"),
            fmt::format("{:.6f}", cot_msg.le));
  EXPECT_EQ(point_element->getAttribute("hae"),
            fmt::format("{:.6f}", cot_msg.hae));
  EXPECT_EQ(point_element->getAttribute("ce"),
            fmt::format("{:.6f}", cot_msg.ce));

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

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
