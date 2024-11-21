#include "gtest/gtest.h"
#include "../src/http/RqTarget.h"
#include <iostream>

TEST(BasicTests, 1_Test) {
    http::RqTarget RqTarget = http::RqTarget("/path/to/file?a=20&b=30#fragment");
    EXPECT_EQ(http::RqTarget(RqTarget.path(), RqTarget.query(), RqTarget.fragment()), RqTarget);
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(BasicTests, 2_Test) {
    http::RqTarget RqTarget = http::RqTarget("/path/to/file?a=20&b=30#fragment");
    std::cout << RqTarget.ToStr() << std::endl;
    std::cout << "path: " << RqTarget.path() << std::endl;
    std::cout << "query: " << RqTarget.query() << std::endl;
    std::cout << "fragment: " << RqTarget.fragment() << std::endl;
    EXPECT_EQ("/path/to/file", RqTarget.path());
    EXPECT_EQ("a=20&b=30", RqTarget.query());
    EXPECT_EQ("fragment", RqTarget.fragment());
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(BasicTests, 3_Test) {
    http::RqTarget RqTarget = http::RqTarget("/path/to/file?a=20&b=30#fragment");
    EXPECT_EQ("/path/to/file?a=20&b=30#fragment", RqTarget.ToStr());
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(BasicTests, 4_Test) {
    http::RqTarget RqTarget = http::RqTarget("path/to/file?a=20&b=30#fragment");
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(BasicTests, 5_Test) {
    http::RqTarget RqTarget = http::RqTarget("");
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(BasicTests, 6_Test) {
    http::RqTarget RqTarget = http::RqTarget(".");
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(BasicTests, 7_Test) {
    http::RqTarget RqTarget = http::RqTarget("/");
    EXPECT_EQ(RqTarget.path(), "/");
    EXPECT_EQ(RqTarget.query(), "");
    EXPECT_EQ(RqTarget.fragment(), "");
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(BasicTests, 8_Test) {
    http::RqTarget RqTarget = http::RqTarget("?");
    EXPECT_EQ(RqTarget.path(), "");
    EXPECT_EQ(RqTarget.query(), "");
    EXPECT_EQ(RqTarget.fragment(), "");
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(BasicTests, 9_Test) {
    http::RqTarget RqTarget = http::RqTarget("#");
    EXPECT_EQ(RqTarget.path(), "");
    EXPECT_EQ(RqTarget.query(), "");
    EXPECT_EQ(RqTarget.fragment(), "");
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(BasicTests, 10_Test) {
    http::RqTarget RqTarget = http::RqTarget(" ");
    EXPECT_EQ(RqTarget.path(), "");
    EXPECT_EQ(RqTarget.query(), "");
    EXPECT_EQ(RqTarget.fragment(), "");
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(BasicTests, 11_Test) {
    http::RqTarget RqTarget = http::RqTarget("abc");
    EXPECT_EQ(RqTarget.path(), "");
    EXPECT_EQ(RqTarget.query(), "");
    EXPECT_EQ(RqTarget.fragment(), "");
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(PathTests, 1_Test) {
    http::RqTarget RqTarget = http::RqTarget("[");
    EXPECT_EQ(RqTarget.path(), "");
    EXPECT_EQ(RqTarget.query(), "");
    EXPECT_EQ(RqTarget.fragment(), "");
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(PathTests, 2_Test) {
    http::RqTarget RqTarget = http::RqTarget("\"");
    EXPECT_EQ(RqTarget.path(), "");
    EXPECT_EQ(RqTarget.query(), "");
    EXPECT_EQ(RqTarget.fragment(), "");
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(QueryTests, 1_Test) {
    http::RqTarget RqTarget = http::RqTarget("/?");
    std::cout << "query: " << RqTarget.query() << std::endl;
    std::cout << "status: " << RqTarget.status() << std::endl;
    EXPECT_EQ(RqTarget.path(), "/");
    EXPECT_EQ(RqTarget.query(), "");
    EXPECT_EQ(RqTarget.fragment(), "");
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(QueryTests, 2_Test) {
    http::RqTarget RqTarget = http::RqTarget("/path?a=20&b=30&c=40");
    std::cout << "query: " << RqTarget.query() << std::endl;
    std::cout << "status: " << RqTarget.status() << std::endl;
    EXPECT_EQ(RqTarget.path(), "/path");
    EXPECT_EQ(RqTarget.query(), "a=20&b=30&c=40");
    EXPECT_EQ(RqTarget.fragment(), "");
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(FragmentTests, 1_Test) {
    http::RqTarget RqTarget = http::RqTarget("/path#top");
    EXPECT_EQ(RqTarget.path(), "/path");
    EXPECT_EQ(RqTarget.query(), "");
    EXPECT_EQ(RqTarget.fragment(), "top");
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(FragmentTests, 2_Test) {
    http::RqTarget RqTarget = http::RqTarget("/path?#page=4");
    EXPECT_EQ(RqTarget.path(), "/path");
    EXPECT_EQ(RqTarget.query(), "");
    EXPECT_EQ(RqTarget.fragment(), "page=4");
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(FragmentTests, 3_Test) {
    http::RqTarget RqTarget = http::RqTarget("/path#\"");
    EXPECT_EQ(RqTarget.path(), "/path");
    EXPECT_EQ(RqTarget.query(), "");
    EXPECT_EQ(RqTarget.fragment(), "\"");
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(CopyTests, 1_Test_Copy_Ctor) {
    http::RqTarget uri1 = http::RqTarget("/path/to/file");
    http::RqTarget uri2 = uri1;
    http::RqTarget uri3(uri1);
    http::RqTarget uri4("/path/to/file2");
    EXPECT_EQ(uri1, uri2);
    EXPECT_EQ(uri1, uri3);
    EXPECT_EQ(uri2, uri3);
    EXPECT_NE(uri1, uri4);
}
