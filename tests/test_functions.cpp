#include <gtest/gtest.h>
#include "taktile/functions.hpp"

TEST(Functions, ConstructCotURL_Default) {
    /// Test case that checks default-constructed URL is the same as the default CoT URL 
    auto result = taktile::URL();
    auto expected = taktile::URL(taktile::DEFAULT_COT_URL);
    EXPECT_EQ(result.scheme, expected.scheme);
    EXPECT_EQ(result.net_loc, expected.net_loc);
    EXPECT_EQ(result.port, expected.port);
}

TEST(Functions, ConstructCotURL_TCPNoPort) {
    /// Test case for constructing a URL with the TCP scheme and no port
    auto result = taktile::URL("tcp://www.example.com");
    auto expected = taktile::URL{taktile::Scheme::TCP, "www.example.com", taktile::DEFAULT_COT_PORT};
    EXPECT_EQ(result.scheme, expected.scheme);
    EXPECT_EQ(result.net_loc, expected.net_loc);
    EXPECT_EQ(result.port, expected.port);
}

TEST(Functions, ConstructCotURL_TLSNoPort) {
    /// Test case for constructing a URL with the TLS scheme and no port
    auto result = taktile::URL("tls://www.example.com");
    auto expected = taktile::URL{taktile::Scheme::TLS, "www.example.com", taktile::DEFAULT_COT_PORT};
    EXPECT_EQ(result.scheme, expected.scheme);
    EXPECT_EQ(result.net_loc, expected.net_loc);
    EXPECT_EQ(result.port, expected.port);
}

TEST(Functions, ConstructCotURL_UDPPort) {
    /// Test case for constructing a URL with the UDP scheme and a port
    auto result = taktile::URL("udp://www.example.com:9999");
    auto expected = taktile::URL{taktile::Scheme::UDP, "www.example.com", 9999};
    EXPECT_EQ(result.scheme, expected.scheme);
    EXPECT_EQ(result.net_loc, expected.net_loc);
    EXPECT_EQ(result.port, expected.port);
}

TEST(Functions, ConstructCotURL_UDPBroadcastNoPort) {
    /// Test case for constructing a URL with the UDP+BROADCAST scheme and no port
    auto result = taktile::URL("udp+broadcast://www.example.com");
    auto expected = taktile::URL{taktile::Scheme::UDP_BROADCAST, "www.example.com", taktile::DEFAULT_BROADCAST_PORT};
    EXPECT_EQ(result.scheme, expected.scheme);
    EXPECT_EQ(result.net_loc, expected.net_loc);
    EXPECT_EQ(result.port, expected.port);
}

TEST(Function, ConstructCotURL_InvalidSchemeThrows) {
    /// Test case for constructing a URL with invalid scheme.
    EXPECT_THROW(taktile::URL("www.example.com"), std::invalid_argument);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}