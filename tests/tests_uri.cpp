#include "gtest/gtest.h"
#include "../src/http/http.h"


TEST(Suite1, 1_Test) {
    http::Uri uri = http::Uri("www.example.com");
    ASSERT_EQ(http::Uri("", "www.example.com", 0, "", "", ""), uri);
}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
