#include "gtest/gtest.h"
#include "../src/http/http.h"
#include <iostream>

TEST(BasicTests, 1_Test) {
    http::Uri uri = http::Uri("www.example.com");
    //std::cout << uri.ToStr() << std::endl;
    ASSERT_EQ("", uri.scheme());
    ASSERT_EQ("www.example.com", uri.host());
    ASSERT_EQ(0, uri.port());
    ASSERT_EQ("", uri.path());
    ASSERT_EQ("", uri.query());
    ASSERT_EQ("", uri.fragment());
    ASSERT_EQ(http::Uri("", "www.example.com", 0, "", "", ""), uri);
}

TEST(CopyTests, 1_Test_Copy_Ctor) {
    http::Uri uri1 = http::Uri("www.example.com");
    http::Uri uri2 = uri1;
    http::Uri uri3(uri1);
    http::Uri uri4("www.example2.com");
    ASSERT_EQ(uri1, uri2);
    ASSERT_EQ(uri1, uri3);
    ASSERT_EQ(uri2, uri3);
    ASSERT_NE(uri1, uri4);
}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
