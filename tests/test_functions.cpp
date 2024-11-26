#include <gtest/gtest.h>
#include "taktile/functions.hpp"

// Example test case
TEST(Functions, ParseURL_HttpsWithPort) {
    auto result = taktile::parse_url("https://example.com:80");
    auto expected = taktile::url{"https", "example.com", 80};
    EXPECT_EQ(result.scheme, expected.scheme);
    EXPECT_EQ(result.netloc, expected.netloc);
    EXPECT_EQ(result.port, expected.port);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}