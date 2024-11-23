#include "gtest/gtest.h"
#include "../src/http/RqTarget.h"
#include <iostream>

TEST(BasicTests, 1_Test) {
    http::RqTarget RqTarget = http::RqTarget("/path/to/file?a=20&b=30#fragment");
    EXPECT_EQ(http::RqTarget(RqTarget.scheme(), RqTarget.user_info(), RqTarget.host(), RqTarget.port(), RqTarget.path(), RqTarget.query(), RqTarget.fragment()), RqTarget);
    EXPECT_EQ(RqTarget.ToStr(), "/path/to/file?a=20&b=30#fragment");
    EXPECT_EQ(RqTarget.scheme(), "");
    EXPECT_EQ(RqTarget.user_info(), "");
    EXPECT_EQ(RqTarget.host(), "");
    EXPECT_EQ(RqTarget.port(), "");
    EXPECT_EQ(RqTarget.path(), "/path/to/file");
    EXPECT_EQ(RqTarget.query(), "a=20&b=30");
    EXPECT_EQ(RqTarget.fragment(), "fragment");
    EXPECT_FALSE(RqTarget.HasScheme());
    EXPECT_FALSE(RqTarget.HasUserInfo());
    EXPECT_FALSE(RqTarget.HasHost());
    EXPECT_FALSE(RqTarget.HasPort());
    EXPECT_TRUE(RqTarget.HasPath());
    EXPECT_TRUE(RqTarget.HasQuery());
    EXPECT_TRUE(RqTarget.HasFragment());
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_SCHEME);
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_HAS_USER_INFO);
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_HOST);
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PORT);
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_QUERY);
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_HAS_FRAGMENT);
    EXPECT_EQ(RqTarget.Good(), false);

}

TEST(BasicTests, 2_Test) {
    http::RqTarget RqTarget = http::RqTarget("/path/to/file?a=20&b=30#fragment");
    EXPECT_EQ("/path/to/file?a=20&b=30#fragment", RqTarget.ToStr());
    EXPECT_EQ("", RqTarget.scheme());
    EXPECT_EQ("", RqTarget.user_info());
    EXPECT_EQ("", RqTarget.host());
    EXPECT_EQ("", RqTarget.port());
    EXPECT_EQ("/path/to/file", RqTarget.path());
    EXPECT_EQ("a=20&b=30", RqTarget.query());
    EXPECT_EQ("fragment", RqTarget.fragment());
    EXPECT_FALSE(RqTarget.HasScheme());
    EXPECT_FALSE(RqTarget.HasUserInfo());
    EXPECT_FALSE(RqTarget.HasHost());
    EXPECT_FALSE(RqTarget.HasPort());
    EXPECT_TRUE(RqTarget.HasPath());
    EXPECT_TRUE(RqTarget.HasQuery());
    EXPECT_TRUE(RqTarget.HasFragment());
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_SCHEME);
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_HAS_USER_INFO);
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_HOST);
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PORT);
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_QUERY);
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_HAS_FRAGMENT);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(BasicTests, 3_Test) {
    http::RqTarget RqTarget = http::RqTarget("http://host:1234/path/to/file?somequery");
    EXPECT_EQ("http://host:1234/path/to/file?somequery", RqTarget.ToStr());
    EXPECT_EQ("http", RqTarget.scheme());
    EXPECT_EQ("", RqTarget.user_info());
    EXPECT_EQ("host", RqTarget.host());
    EXPECT_EQ("1234", RqTarget.port());
    EXPECT_EQ("/path/to/file", RqTarget.path());
    EXPECT_EQ("somequery", RqTarget.query());
    EXPECT_EQ("", RqTarget.fragment());
    EXPECT_TRUE(RqTarget.HasScheme());
    EXPECT_FALSE(RqTarget.HasUserInfo());
    EXPECT_TRUE(RqTarget.HasHost());
    EXPECT_TRUE(RqTarget.HasPort());
    EXPECT_TRUE(RqTarget.HasPath());
    EXPECT_TRUE(RqTarget.HasQuery());
    EXPECT_FALSE(RqTarget.HasFragment());
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(BasicTests, 4_Test) {
    http::RqTarget RqTarget = http::RqTarget("http://user:password@www.test.com/?");
    EXPECT_EQ("http://user:password@www.test.com/?", RqTarget.ToStr());
    EXPECT_EQ("http", RqTarget.scheme());
    EXPECT_EQ("user:password", RqTarget.user_info());
    EXPECT_EQ("www.test.com", RqTarget.host());
    EXPECT_EQ("", RqTarget.port());
    EXPECT_EQ("/", RqTarget.path());
    EXPECT_EQ("", RqTarget.query());
    EXPECT_EQ("", RqTarget.fragment());
    EXPECT_TRUE(RqTarget.HasScheme());
    EXPECT_TRUE(RqTarget.HasUserInfo());
    EXPECT_TRUE(RqTarget.HasHost());
    EXPECT_FALSE(RqTarget.HasPort());
    EXPECT_TRUE(RqTarget.HasPath());
    EXPECT_TRUE(RqTarget.HasQuery());
    EXPECT_FALSE(RqTarget.HasFragment());
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
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(BasicTests, 8_Test) {
    http::RqTarget RqTarget = http::RqTarget("?");
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(BasicTests, 9_Test) {
    http::RqTarget RqTarget = http::RqTarget("#");
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(BasicTests, 10_Test) {
    http::RqTarget RqTarget = http::RqTarget(" ");
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
    EXPECT_EQ(RqTarget.path(), "/");
    EXPECT_EQ(RqTarget.query(), "");
    EXPECT_EQ(RqTarget.fragment(), "");
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(QueryTests, 2_Test) {
    http::RqTarget RqTarget = http::RqTarget("/path?a=20&b=30&c=40");
    std::cout << "query: " << RqTarget.query() << std::endl;
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
