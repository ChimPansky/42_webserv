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
    std::cout << uri.ToStr() << std::endl;
    std::cout << "path: " << uri.path() << std::endl;
    std::cout << "query: " << uri.query() << std::endl;
    std::cout << "fragment: " << uri.fragment() << std::endl;
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

TEST(PathTests, 1_Test) {
    http::Uri uri = http::Uri("[");
    ASSERT_EQ(uri.path(), "");
    ASSERT_EQ(uri.query(), "");
    ASSERT_EQ(uri.fragment(), "");
    ASSERT_EQ(uri.Good(), false);
}

TEST(PathTests, 2_Test) {
    http::Uri uri = http::Uri("\"");
    ASSERT_EQ(uri.path(), "");
    ASSERT_EQ(uri.query(), "");
    ASSERT_EQ(uri.fragment(), "");
    ASSERT_EQ(uri.Good(), false);
}

TEST(QueryTests, 1_Test) {
    http::Uri uri = http::Uri("/?");
    std::cout << "query: " << uri.query() << std::endl;
    std::cout << "status: " << uri.status() << std::endl;
    ASSERT_EQ(uri.path(), "/");
    ASSERT_EQ(uri.query(), "");
    ASSERT_EQ(uri.fragment(), "");
    ASSERT_EQ(uri.Good(), false);
}

TEST(QueryTests, 2_Test) {
    http::Uri uri = http::Uri("/path?!");
    std::cout << "query: " << uri.query() << std::endl;
    std::cout << "status: " << uri.status() << std::endl;
    ASSERT_EQ(uri.path(), "/path");
    ASSERT_EQ(uri.query(), "!");
    ASSERT_EQ(uri.fragment(), "");
    ASSERT_EQ(uri.Good(), false);
}

TEST(FragmentTests, 1_Test) {
    http::Uri uri = http::Uri("/path#\"");
    ASSERT_EQ(uri.path(), "/path");
    ASSERT_EQ(uri.query(), "");
    ASSERT_EQ(uri.fragment(), "\"");
    ASSERT_EQ(uri.Good(), false);
}

TEST(FragmentTests, 2_Test) {
    http::Uri uri = http::Uri("/path?#invalid!");
    ASSERT_EQ(uri.path(), "/path");
    ASSERT_EQ(uri.query(), "");
    ASSERT_EQ(uri.fragment(), "invalid!");
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
