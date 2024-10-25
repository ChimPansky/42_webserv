#include "gtest/gtest.h"
#include "../src/http/Uri.h"
#include <iostream>

TEST(BasicTests, 1_Test) {
    http::Uri uri = http::Uri("/path/to/file?a=20&b=30#fragment");
    ASSERT_EQ(http::Uri(uri.path(), uri.query(), uri.fragment()), uri);
    ASSERT_EQ(uri.Good(), true);
}

TEST(BasicTests, 2_Test) {
    http::Uri uri = http::Uri("/path/to/file?a=20&b=30#fragment");
    //std::cout << uri.ToStr() << std::endl;
    ASSERT_EQ("/path/to/file", uri.path());
    ASSERT_EQ("a=20&b=30", uri.query());
    ASSERT_EQ("fragment", uri.fragment());
    ASSERT_EQ(uri.Good(), true);
}

TEST(BasicTests, 3_Test) {
    http::Uri uri = http::Uri("/path/to/file?a=20&b=30#fragment");
    ASSERT_EQ("/path/to/file?a=20&b=30#fragment", uri.ToStr());
    ASSERT_EQ(uri.Good(), true);
}

TEST(BasicTests, 4_Test) {
    http::Uri uri = http::Uri("path/to/file?a=20&b=30#fragment");
    ASSERT_EQ(uri.Good(), false);
}

TEST(BasicTests, 5_Test) {
    http::Uri uri = http::Uri("");
    ASSERT_EQ(uri.Good(), false);
}

TEST(BasicTests, 6_Test) {
    http::Uri uri = http::Uri(".");
    ASSERT_EQ(uri.Good(), false);
}

TEST(BasicTests, 7_Test) {
    http::Uri uri = http::Uri("/");
    ASSERT_EQ(uri.path(), "/");
    ASSERT_EQ(uri.query(), "");
    ASSERT_EQ(uri.fragment(), "");
    ASSERT_EQ(uri.Good(), true);
}

TEST(BasicTests, 8_Test) {
    http::Uri uri = http::Uri("?");
    ASSERT_EQ(uri.path(), "");
    ASSERT_EQ(uri.query(), "");
    ASSERT_EQ(uri.fragment(), "");
    ASSERT_EQ(uri.Good(), false);
}

TEST(BasicTests, 9_Test) {
    http::Uri uri = http::Uri("#");
    ASSERT_EQ(uri.path(), "");
    ASSERT_EQ(uri.query(), "");
    ASSERT_EQ(uri.fragment(), "");
    ASSERT_EQ(uri.Good(), false);
}

TEST(BasicTests, 10_Test) {
    http::Uri uri = http::Uri(" ");
    ASSERT_EQ(uri.path(), "");
    ASSERT_EQ(uri.query(), "");
    ASSERT_EQ(uri.fragment(), "");
    ASSERT_EQ(uri.Good(), false);
}

TEST(BasicTests, 11_Test) {
    http::Uri uri = http::Uri("abc");
    ASSERT_EQ(uri.path(), "");
    ASSERT_EQ(uri.query(), "");
    ASSERT_EQ(uri.fragment(), "");
    ASSERT_EQ(uri.Good(), false);
}

TEST(CopyTests, 1_Test_Copy_Ctor) {
    http::Uri uri1 = http::Uri("/path/to/file");
    http::Uri uri2 = uri1;
    http::Uri uri3(uri1);
    http::Uri uri4("/path/to/file2");
    ASSERT_EQ(uri1, uri2);
    ASSERT_EQ(uri1, uri3);
    ASSERT_EQ(uri2, uri3);
    ASSERT_NE(uri1, uri4);
}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
