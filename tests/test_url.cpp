// Copyright (c) 2025, Joe Dinius, Ph.D.
// SPDX-License-Identifier: Apache-2.0
#include <gtest/gtest.h>
#include "taktile/url.hpp"

TEST(URL, DefaultConstructor) {
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

TEST(URL, ConstructHttpWithDefaultPort) {
  /// Test case for constructing a URL with the HTTP scheme and no port
  auto result = taktile::URL("http://www.example.com");
  auto expected = taktile::URL{taktile::Scheme::HTTP, "www.example.com",
                               taktile::DEFAULT_COT_PORT};
  EXPECT_EQ(result.scheme, expected.scheme);
  EXPECT_EQ(result.net_loc, expected.net_loc);
  EXPECT_EQ(result.port, expected.port);
}

TEST(URL, ConstructHttpsWithDefaultPort) {
  /// Test case for constructing a URL with the HTTPS scheme and no port
  auto result = taktile::URL("https://www.example.com");
  auto expected = taktile::URL{taktile::Scheme::HTTPS, "www.example.com",
                               taktile::DEFAULT_COT_PORT};
  EXPECT_EQ(result.scheme, expected.scheme);
  EXPECT_EQ(result.net_loc, expected.net_loc);
  EXPECT_EQ(result.port, expected.port);
}

TEST(URL, ConstructTcpWithDefaultPort) {
  /// Test case for constructing a URL with the TCP scheme and no port
  auto result = taktile::URL("tcp://www.example.com");
  auto expected = taktile::URL{taktile::Scheme::TCP, "www.example.com",
                               taktile::DEFAULT_COT_PORT};
  EXPECT_EQ(result.scheme, expected.scheme);
  EXPECT_EQ(result.net_loc, expected.net_loc);
  EXPECT_EQ(result.port, expected.port);
}

TEST(URL, ConstructTlsWithDefaultPort) {
  /// Test case for constructing a URL with the TLS scheme and no port
  auto result = taktile::URL("tls://www.example.com");
  auto expected = taktile::URL{taktile::Scheme::TLS, "www.example.com",
                               taktile::DEFAULT_COT_PORT};
  EXPECT_EQ(result.scheme, expected.scheme);
  EXPECT_EQ(result.net_loc, expected.net_loc);
  EXPECT_EQ(result.port, expected.port);
}

TEST(URL, ConstructUdpWithPort) {
  /// Test case for constructing a URL with the UDP scheme and a port
  auto result = taktile::URL("udp://www.example.com:9999");
  auto expected = taktile::URL{taktile::Scheme::UDP, "www.example.com", 9999};
  EXPECT_EQ(result.scheme, expected.scheme);
  EXPECT_EQ(result.net_loc, expected.net_loc);
  EXPECT_EQ(result.port, expected.port);
}

TEST(URL, ConstructUdpBroadcastWithPort) {
  /// Test case for constructing a URL with the UDP+BROADCAST scheme and no port
  auto result = taktile::URL("udp+broadcast://www.example.com");
  auto expected =
      taktile::URL{taktile::Scheme::UDP_BROADCAST, "www.example.com",
                   taktile::DEFAULT_BROADCAST_PORT};
  EXPECT_EQ(result.scheme, expected.scheme);
  EXPECT_EQ(result.net_loc, expected.net_loc);
  EXPECT_EQ(result.port, expected.port);
}

TEST(URL, InvalidURL) {
  /// Test case for constructing a URL with invalid scheme.
  EXPECT_THROW(taktile::URL("www.example.com"), std::invalid_argument);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
