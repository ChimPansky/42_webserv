#include "gtest/gtest.h"
#include "../src/http/Uri.h"
#include <iostream>

TEST(BasicTests, 1_Test) {
    http::Uri uri = http::Uri("/path/to/file?a=20&b=30#fragment");
    EXPECT_EQ(http::Uri(uri.path(), uri.query(), uri.fragment()), uri);
    EXPECT_EQ(uri.Good(), true);
}

TEST(BasicTests, 2_Test) {
    http::Uri uri = http::Uri("/path/to/file?a=20&b=30#fragment");
    std::cout << uri.ToStr() << std::endl;
    std::cout << "path: " << uri.path() << std::endl;
    std::cout << "query: " << uri.query() << std::endl;
    std::cout << "fragment: " << uri.fragment() << std::endl;
    EXPECT_EQ("/path/to/file", uri.path());
    EXPECT_EQ("a=20&b=30", uri.query());
    EXPECT_EQ("fragment", uri.fragment());
    EXPECT_EQ(uri.Good(), true);
}

TEST(BasicTests, 3_Test) {
    http::Uri uri = http::Uri("/path/to/file?a=20&b=30#fragment");
    EXPECT_EQ("/path/to/file?a=20&b=30#fragment", uri.ToStr());
    EXPECT_EQ(uri.Good(), true);
}

TEST(BasicTests, 4_Test) {
    http::Uri uri = http::Uri("path/to/file?a=20&b=30#fragment");
    EXPECT_EQ(uri.Good(), false);
}

TEST(BasicTests, 5_Test) {
    http::Uri uri = http::Uri("");
    EXPECT_EQ(uri.Good(), false);
}

TEST(BasicTests, 6_Test) {
    http::Uri uri = http::Uri(".");
    EXPECT_EQ(uri.Good(), false);
}

TEST(BasicTests, 7_Test) {
    http::Uri uri = http::Uri("/");
    EXPECT_EQ(uri.path(), "/");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), true);
}

TEST(BasicTests, 8_Test) {
    http::Uri uri = http::Uri("?");
    EXPECT_EQ(uri.path(), "");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), false);
}

TEST(BasicTests, 9_Test) {
    http::Uri uri = http::Uri("#");
    EXPECT_EQ(uri.path(), "");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), false);
}

TEST(BasicTests, 10_Test) {
    http::Uri uri = http::Uri(" ");
    EXPECT_EQ(uri.path(), "");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), false);
}

TEST(BasicTests, 11_Test) {
    http::Uri uri = http::Uri("abc");
    EXPECT_EQ(uri.path(), "");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), false);
}

TEST(PathTests, 1_Test) {
    http::Uri uri = http::Uri("[");
    EXPECT_EQ(uri.path(), "");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), false);
}

TEST(PathTests, 2_Test) {
    http::Uri uri = http::Uri("\"");
    EXPECT_EQ(uri.path(), "");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), false);
}

TEST(QueryTests, 1_Test) {
    http::Uri uri = http::Uri("/?");
    std::cout << "query: " << uri.query() << std::endl;
    std::cout << "status: " << uri.status() << std::endl;
    EXPECT_EQ(uri.path(), "/");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), false);
}

TEST(QueryTests, 2_Test) {
    http::Uri uri = http::Uri("/path?a=20&b=30&c=40");
    std::cout << "query: " << uri.query() << std::endl;
    std::cout << "status: " << uri.status() << std::endl;
    EXPECT_EQ(uri.path(), "/path");
    EXPECT_EQ(uri.query(), "a=20&b=30&c=40");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), true);
}

TEST(FragmentTests, 1_Test) {
    http::Uri uri = http::Uri("/path#top");
    EXPECT_EQ(uri.path(), "/path");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "top");
    EXPECT_EQ(uri.Good(), true);
}

TEST(FragmentTests, 2_Test) {
    http::Uri uri = http::Uri("/path?#page=4");
    EXPECT_EQ(uri.path(), "/path");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "page=4");
    EXPECT_EQ(uri.Good(), true);
}

TEST(FragmentTests, 3_Test) {
    http::Uri uri = http::Uri("/path#\"");
    EXPECT_EQ(uri.path(), "/path");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "\"");
    EXPECT_EQ(uri.Good(), true);
}

TEST(CopyTests, 1_Test_Copy_Ctor) {
    http::Uri uri1 = http::Uri("/path/to/file");
    http::Uri uri2 = uri1;
    http::Uri uri3(uri1);
    http::Uri uri4("/path/to/file2");
    EXPECT_EQ(uri1, uri2);
    EXPECT_EQ(uri1, uri3);
    EXPECT_EQ(uri2, uri3);
    EXPECT_NE(uri1, uri4);
}
